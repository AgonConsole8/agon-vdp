# 1 "C:\\Users\\restd\\AppData\\Local\\Temp\\tmpfognni94"
#include <Arduino.h>
# 1 "D:/Console8/agon-vdp-otf/video/video.ino"
# 48 "D:/Console8/agon-vdp-otf/video/video.ino"
#include <WiFi.h>
#include <HardwareSerial.h>
#include <fabgl.h>

#define DEBUG 1
#define SERIALBAUDRATE 115200

HardwareSerial DBGSerial(0);

#include "agon.h"

TerminalState terminalState = TerminalState::Disabled;
bool consoleMode = false;
bool printerOn = false;

#include "version.h"
#include "agon_ps2.h"
#include "agon_audio.h"
#include "agon_ttxt.h"
#include "graphics.h"
#include "cursor.h"
#include "vdp_protocol.h"
#include "vdu_stream_processor.h"
#include "hexload.h"

std::unique_ptr<fabgl::Terminal> Terminal;
VDUStreamProcessor * processor;

#include "zdi.h"

extern void bdpp_run_test();
void setup();
void loop();
void do_keyboard();
void do_keyboard_terminal();
void do_mouse();
void boot_screen();
void debug_log(const char *format, ...);
void setConsoleMode(bool mode);
void startTerminal();
void stopTerminal();
void suspendTerminal();
bool processTerminal();
void print(char const * text);
void printFmt(const char *format, ...);
#line 80 "D:/Console8/agon-vdp-otf/video/video.ino"
void setup() {
 disableCore0WDT(); delay(200);
 disableCore1WDT(); delay(200);
 DBGSerial.begin(SERIALBAUDRATE, SERIAL_8N1, 3, 1);
 copy_font();
 set_mode(1);
 setupVDPProtocol();
 processor = new VDUStreamProcessor(&VDPSerial);
 initAudio();
 processor->wait_eZ80();
 setupKeyboardAndMouse();
 resetMousePositioner(canvasW, canvasH, _VGAController.get());
 processor->sendModeInformation();
 boot_screen();

 bdpp_run_test();
}



void loop() {
 bool drawCursor = false;
 auto cursorTime = millis();

 while (true) {
  if (processTerminal()) {
   continue;
  }
  if (millis() - cursorTime > CURSOR_PHASE) {
   cursorTime = millis();
   drawCursor = !drawCursor;
   if (ttxtMode) {
    ttxt_instance.flash(drawCursor);
   }
   do_cursor();
  }
  do_keyboard();
  do_mouse();

  if (processor->byteAvailable()) {
   if (drawCursor) {
    drawCursor = false;
    do_cursor();
   }
   processor->processNext();
  }
 }
}



void do_keyboard() {
 uint8_t keycode;
 uint8_t modifiers;
 uint8_t vk;
 uint8_t down;
 if (getKeyboardKey(&keycode, &modifiers, &vk, &down)) {


  if (down) {
   switch (keycode) {
    case 2:
    case 3:
    case 6:
    case 7:
    case 12:
    case 14 ... 15:
     processor->vdu(keycode);
     break;
    case 16:

     printerOn = !printerOn;
   }
  }


  uint8_t packet[] = {
   keycode,
   modifiers,
   vk,
   down,
  };
  processor->send_packet(PACKET_KEYCODE, sizeof packet, packet);
 }
}



void do_keyboard_terminal() {
 uint8_t ascii;
 if (getKeyboardKey(&ascii)) {

  processor->writeByte(ascii);
 }
}



void do_mouse() {

 MouseDelta delta;
 if (mouseMoved(&delta)) {
  auto mouse = getMouse();
  auto mStatus = mouse->status();

  setMouseCursorPos(mStatus.X, mStatus.Y);
  processor->sendMouseData(&delta);
 }
}



void boot_screen() {
 printFmt("Agon %s VDP Version %d.%d.%d", VERSION_VARIANT, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
 #if VERSION_CANDIDATE > 0
  printFmt(" %s%d", VERSION_TYPE, VERSION_CANDIDATE);
 #endif

 #ifdef VERSION_BUILD
  printFmt(" Build %s", VERSION_BUILD);
 #endif
 printFmt("\n\r");
}



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


   do_keyboard_terminal();
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
   };
   debug_log("Terminal enabled\n\r");
   terminalState = TerminalState::Enabled;
  } break;
  case TerminalState::Enabled: {
   do_keyboard_terminal();


   if (processor->byteAvailable()) {
    Terminal->write(processor->readByte());
   }
  } break;
  case TerminalState::Disabling: {
   Terminal->deactivate();
   Terminal = nullptr;
   set_mode(1);
   processor->sendModeInformation();
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
  processor->vdu(text[i]);
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