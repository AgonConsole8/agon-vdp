# 1 "/tmp/tmpbo45ense"
#include <Arduino.h>
# 1 "/home/curtis/agon/agon-vdp-otf/video/video.ino"
# 48 "/home/curtis/agon/agon-vdp-otf/video/video.ino"
#include <HardwareSerial.h>
#include <WiFi.h>
#include <fabgl.h>
#include <ESP32Time.h>



#ifdef USERSPACE
#undef DEBUG
#define DEBUG 1
#else
#undef DEBUG
#define DEBUG 1
#endif

#define SERIALBAUDRATE 115200

#ifdef USERSPACE
extern uint32_t startup_screen_mode;
#else
#define startup_screen_mode 0
#endif

HardwareSerial DBGSerial(0);

#include "agon.h"

TerminalState terminalState = TerminalState::Disabled;
bool consoleMode = false;
bool printerOn = false;
bool controlKeys = true;
ESP32Time rtc(0);

#include "version.h"
#include "agon_ps2.h"
#include "agon_audio.h"
#include "agon_screen.h"
#include "agon_ttxt.h"
#include "vdp_protocol.h"
#include "vdu_stream_processor.h"
#include "hexload.h"

std::unique_ptr<fabgl::Terminal> Terminal;
VDUStreamProcessor * processor;

#ifndef USERSPACE
#include "zdi.h"
#endif

TaskHandle_t Core0Task;
void setup();
void loop();
void processLoop(void * parameter);
void boot_screen();
void debug_log(const char *format, ...);
void force_debug_log(const char *format, ...);
void setConsoleMode(bool mode);
void startTerminal();
void stopTerminal();
void suspendTerminal();
bool processTerminal();
void print(char const * text);
void printFmt(const char *format, ...);
#line 99 "/home/curtis/agon/agon-vdp-otf/video/video.ino"
void setup() {
 #ifndef VDP_USE_WDT
  disableCore0WDT(); delay(200);
  disableCore1WDT(); delay(200);
 #endif
 DBGSerial.begin(SERIALBAUDRATE, SERIAL_8N1, 3, 1);
 changeMode(startup_screen_mode);
 copy_font();
 setupVDPProtocol();
 processor = new VDUStreamProcessor(&VDPSerial);
 xTaskCreatePinnedToCore(
  processLoop,
  "processLoop",
  4096,
  NULL,
  3,
  &Core0Task,
  0
 );
 initAudio();
 boot_screen();
 debug_log("Setup ran on core %d, busy core is %d\n\r", xPortGetCoreID(), CoreUsage::busiestCore());
}



void loop() {
 delay(1000);
}

void processLoop(void * parameter) {
#ifdef USERSPACE
 uint32_t count = 0;
#endif

 setupKeyboardAndMouse();
 processor->wait_eZ80();

 while (true) {
#ifdef USERSPACE
   if ((count & 0x7f) == 0) {
   delay(1 );
  }
   count++;
#endif

  #ifdef VDP_USE_WDT
   esp_task_wdt_reset();
  #endif
  if (processTerminal()) {
   continue;
  }

  processor->processNext();
 }
}



void boot_screen() {
 printFmt("Agon %s VDP Version %d.%d.%d", VERSION_VARIANT, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
 #if VERSION_CANDIDATE > 0
  printFmt(" %s%d", VERSION_TYPE, VERSION_CANDIDATE);
 #endif
 #ifdef VERSION_SUBTITLE
  printFmt(" %s", VERSION_SUBTITLE);
 #endif

 #ifdef VERSION_BUILD
  printFmt(" Build %s", VERSION_BUILD);
 #endif
 printFmt("\n\r");
}



#ifndef USERSPACE
void debug_log(const char *format, ...) {
 #if DEBUG == 1
 va_list ap;
 va_start(ap, format);
 auto size = vsnprintf(nullptr, 0, format, ap) + 1;
 if (size > 0) {
  va_end(ap);
  va_start(ap, format);
  char buf[size + 1];
  vsnprintf(buf, size, format, ap);
  DBGSerial.print(buf);
 }
 va_end(ap);
 #endif
}
#endif

void force_debug_log(const char *format, ...) {
 va_list ap;
 va_start(ap, format);
 auto size = vsnprintf(nullptr, 0, format, ap) + 1;
 if (size > 0) {
  va_end(ap);
  va_start(ap, format);
  char buf[size + 1];
  vsnprintf(buf, size, format, ap);
  DBGSerial.print(buf);
 }
 va_end(ap);
}





void setConsoleMode(bool mode) {
 consoleMode = mode;
}



void startTerminal() {
 switch (terminalState) {
  case TerminalState::Disabled: {
   terminalState = TerminalState::Enabling;
  } break;
  case TerminalState::Suspending: {
   terminalState = TerminalState::Enabled;
  } break;
  case TerminalState::Suspended: {
   terminalState = TerminalState::Resuming;
  } break;
 }
}

void stopTerminal() {
 switch (terminalState) {
  case TerminalState::Enabled:
  case TerminalState::Resuming:
  case TerminalState::Suspended:
  case TerminalState::Suspending: {
   terminalState = TerminalState::Disabling;
  } break;
  case TerminalState::Enabling: {
   terminalState = TerminalState::Disabled;
  } break;
 }
}

void suspendTerminal() {
 switch (terminalState) {
  case TerminalState::Enabled:
  case TerminalState::Resuming: {
   terminalState = TerminalState::Suspending;
   processTerminal();
  } break;
  case TerminalState::Enabling: {

   processTerminal();
   terminalState = TerminalState::Suspending;
  } break;
 }
}



bool processTerminal() {
 switch (terminalState) {
  case TerminalState::Disabled: {

   return false;
  } break;
  case TerminalState::Suspended: {

   return false;
  } break;
  case TerminalState::Enabling: {

   Terminal = std::unique_ptr<fabgl::Terminal>(new fabgl::Terminal());
   Terminal->begin(_VGAController.get());
   Terminal->connectSerialPort(VDPSerial);
   Terminal->enableCursor(true);

   Terminal->onVirtualKeyItem = [&](VirtualKeyItem * vkItem) {
    if (vkItem->vk == VirtualKey::VK_F12) {
     if (vkItem->CTRL && (vkItem->LALT || vkItem->RALT)) {

      stopTerminal();
     }
    }
   };


   Terminal->onUserSequence = [&](char const * seq) {

    if (strcmp("Q!", seq) == 0) {
     stopTerminal();
    }
    if (strcmp("S!", seq) == 0) {
     suspendTerminal();
    }
    if (seq[0] == 'F') {
     uint32_t fontnum = textToWord(seq + 1);
     if (fontnum >= 0) {
      auto font = fonts[fontnum];
      if (font != nullptr && font->chptr == nullptr) {
       Terminal->loadFont(font.get());
      }
     }
    }
   };
   debug_log("Terminal enabled\n\r");
   terminalState = TerminalState::Enabled;
  } break;
  case TerminalState::Enabled: {


   if (processor->byteAvailable()) {
    Terminal->write(processor->readByte());
   }
  } break;
  case TerminalState::Disabling: {
   Terminal->deactivate();
   Terminal = nullptr;
   auto context = processor->getContext();

   processor->vdu_mode(videoMode);
   debug_log("Terminal disabled\n\r");
   terminalState = TerminalState::Disabled;
  } break;
  case TerminalState::Suspending: {

   debug_log("Terminal suspended\n\r");
   terminalState = TerminalState::Suspended;
  } break;
  case TerminalState::Resuming: {

   debug_log("Terminal resumed\n\r");
   terminalState = TerminalState::Enabled;
  } break;
  default: {
   debug_log("processTerminal: unknown terminal state %d\n\r", terminalState);
   return false;
  } break;
 }
 return true;
}

void print(char const * text) {
 for (auto i = 0; i < strlen(text); i++) {
  processor->vdu(text[i], false);
 }
}

void printFmt(const char *format, ...) {
 va_list ap;
 va_start(ap, format);
 int size = vsnprintf(nullptr, 0, format, ap) + 1;
 if (size > 0) {
  va_end(ap);
  va_start(ap, format);
  char buf[size + 1];
  vsnprintf(buf, size, format, ap);
  print(buf);
 }
 va_end(ap);
}