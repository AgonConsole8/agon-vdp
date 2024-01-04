#ifndef ON_THE_FLY
#define ON_THE_FLY

#include "src/di_manager.h"
#include <memory>
#include <fabgl.h>
#include "agon.h"
#include "agon_fonts.h"
#include "dispdrivers/vgabasecontroller.h"
#include "src/di_timing.h"

#define OTF_MANAGER_PRIORITY    (configMAX_PRIORITIES - 1) // Task priority for manager for OTF (800x600x64) mode

extern std::unique_ptr<fabgl::Canvas>           	canvas;
extern std::unique_ptr<fabgl::VGABaseController>	_VGAController;
extern uint8_t                                      videoMode;
extern uint8_t                                      _VGAColourDepth;

extern void print(char const * text);
extern int8_t change_mode(uint8_t mode);
extern void stream_send_mode_information();

DiManager* di_manager; // Used for OTF 800x600x64 mode

void otf_task(void * options) {
	debug_log("OTF task running\r\n");
	di_manager = new DiManager();
	di_manager->create_root();

	auto opts = (uint32_t) options;
	if (opts == 1) {
		// Create a solid black rectangle as the screen background.
		OtfCmd_41_Create_primitive_Solid_Rectangle cmd;
		cmd.m_color = PIXEL_ALPHA_100_MASK|0x00; // 100% black
		cmd.m_flags = PRIM_FLAGS_DEFAULT;
		cmd.m_w = otf_video_params.m_active_pixels;
		cmd.m_h = otf_video_params.m_active_lines;
		cmd.m_id = 1;
		cmd.m_pid = 0;
		cmd.m_x = 0;
		cmd.m_y = 0;
		auto rect = di_manager->create_solid_rectangle(&cmd);
		di_manager->generate_code_for_primitive(cmd.m_id);
	} else if (opts == 2) {
		// Create a full screen text area as the screen background.
		OtfCmd_150_Create_primitive_Text_Area cmd;
		cmd.m_flags = PRIM_FLAGS_DEFAULT;
		cmd.m_id = 1;
		cmd.m_pid = ROOT_PRIMITIVE_ID;
		cmd.m_x = 0;
		cmd.m_y = 0;
		cmd.m_columns = otf_video_params.m_active_pixels / 8;
		cmd.m_rows = otf_video_params.m_active_lines / 8;
		cmd.m_bgcolor = PIXEL_ALPHA_100_MASK|0x00;
		cmd.m_fgcolor = PIXEL_ALPHA_100_MASK|0x05;
		auto text_area = di_manager->create_text_area(&cmd, fabgl::FONT_AGON_DATA);
		di_manager->generate_code_for_primitive(cmd.m_id);
	}

	stream_send_mode_information();
	debug_log("Running OTF manager...\r\n");
	di_manager->run();
}

int8_t use_otf_mode(int8_t mode) {
	if (_VGAController) {
		_VGAController->end();
		_VGAController.reset();
	}
	canvas.reset();

	// Modes: 32..47 (0x20..0x2F) - change mode, but create no primitives
	// Modes: 48..63 (0x30..0x3F) - change mode, create full screen black rectangle
	// Modes: 64..79 (0x40..0x4F) - change mode, create full screen text area

    auto options = (mode - 32) / 16;
	auto resolution = (mode & 15);

	// Modeline for 800x600@60Hz resolution, positive syncs
	#define SVGA_800x600_60Hz_Pos "\"800x600@60Hz\" 40 800 840 968 1056 600 601 605 628 +HSync +VSync"

	// Modeline for 684x384@60Hz resolution, opposite syncs
	#define SVGA_684x384_60Hz "\"684x384@60Hz\" 42.75 684 720 792 900 384 385 387 398 -HSync +VSync DoubleScan"

	// Modeline for 1368x768@60Hz resolution, opposite syncs
	#define SVGA_1368x768_60Hz "\"1368x768@60Hz\" 85.5 1368 1440 1584 1800 768 769 772 795 -HSync +VSync"

	// Modeline for 1280x720@60Hz resolution                1280 1468 1604 1664
	#define SVGA_1280x720_60Hz_ADJ "\"1280x720@60Hz\" 74.25 1280 1344 1480 1664 720 721 724 746 +hsync +vsync"

	// Modeline for 640x512@60Hz resolution (for pixel perfect 1280x1024 double scan resolution)
	#define QSVGA_640x512_60Hz_ADJ "\"640x512@60Hz\" 54     640 664 720 844 512 513 515 533 -HSync -VSync DoubleScan"

	// Modeline for 320x200@75Hz resolution
	#define VGA_320x200_75Hz_ADJ "\"320x200@75Hz\" 12.93 336 368 376 408 200 208 211 229 -HSync -VSync DoubleScan"

	// Modeline for 1024x768@60Hz resolution
	#define SVGA_1024x768_60Hz_ADJ "\"1024x768@60Hz\" 65 1024 1056 1192 1344 768 771 777 806 -HSync -VSync"

	// Modeline for 320x200@70Hz resolution - the same of VGA_640x200_70Hz with horizontal halved
	#define VGA_320x200_70Hz_ADJ "\"320x200@70Hz\" 12.5875 320 328 356 400 200 206 207 224 -HSync -VSync DoubleScan"

	const char* mode_line = NULL;
	switch (resolution) {
		case 0: mode_line = SVGA_800x600_60Hz_Pos; break; // (100x75) good
		case 1: mode_line = SVGA_800x600_60Hz; break; // (100x75) good
		case 2: mode_line = SVGA_684x384_60Hz; break; // (85x48) quarter of 1368x768, fuzzy
		case 3: mode_line = QSVGA_640x512_60Hz_ADJ; break; // (80x64) quarter of 1280x1024, clean, but missing 4 rightmost columns
		case 4: mode_line = VGA_640x480_60Hz; break; // (80x60) good
		case 5: mode_line = VGA_640x240_60Hz; break; // (80x30) clean, but missing 2 rightmost columns
		case 6: mode_line = VGA_512x384_60Hz; break; // (64x48) quarter of 1024x768, good
		case 7: mode_line = QVGA_320x240_60Hz; break; // (40x30) quarter of 640x480, clean, but missing 1 rightmost column
		case 8: mode_line = VGA_320x200_75Hz; break; // (40x25) clean, but missing 1 rightmost column
		case 9: mode_line = VGA_320x200_70Hz_ADJ; break; // (40x25) clean, but missing 2 rightmost columns
		case 10: mode_line = SVGA_1024x768_60Hz_ADJ; break; // (128x96) good
		case 11: mode_line = SVGA_1280x720_60Hz_ADJ; break; // (160x90) clean, but missing columns 134 to 159
		case 12: mode_line = SVGA_1368x768_60Hz; break; // out of range on monitor
	}

	if (!mode_line) {
		return -1; // invalid mode
	}

	fabgl::VGATimings timings;
	if (!fabgl::VGABaseController::convertModelineToTimings(mode_line, &timings)) {
		return -1; // invalid mode
	}

	otf_video_params.m_mode_line = mode_line;
	otf_video_params.m_scan_count = timings.scanCount;
	otf_video_params.m_active_lines = timings.VVisibleArea;
    otf_video_params.m_vfp_lines = timings.VFrontPorch;
    otf_video_params.m_vs_lines = timings.VSyncPulse;
    otf_video_params.m_vbp_lines = timings.VBackPorch;
    otf_video_params.m_hfp_pixels = timings.HFrontPorch;
    otf_video_params.m_hs_pixels = timings.HSyncPulse;
    otf_video_params.m_active_pixels = timings.HVisibleArea;
    otf_video_params.m_hbp_pixels = timings.HBackPorch;
    otf_video_params.m_dma_clock_freq = timings.frequency;
    otf_video_params.m_dma_total_lines = timings.VVisibleArea * timings.scanCount + timings.VFrontPorch + timings.VSyncPulse + timings.VBackPorch;
    otf_video_params.m_dma_total_descr = otf_video_params.m_dma_total_lines;
    otf_video_params.m_hs_on = (timings.HSyncLogic == '+' ? 1 : 0) << VGA_HSYNC_BIT;
    otf_video_params.m_hs_off = (timings.HSyncLogic == '+' ? 0 : 1) << VGA_HSYNC_BIT;
    otf_video_params.m_vs_on = (timings.VSyncLogic == '+' ? 1 : 0) << VGA_VSYNC_BIT;
    otf_video_params.m_vs_off = (timings.VSyncLogic == '+' ? 0 : 1) << VGA_VSYNC_BIT;
    otf_video_params.m_syncs_on = otf_video_params.m_hs_on | otf_video_params.m_vs_on;
    otf_video_params.m_syncs_off = otf_video_params.m_hs_off | otf_video_params.m_vs_off;
    otf_video_params.m_syncs_off_x4 =
		(otf_video_params.m_syncs_off << 24) | (otf_video_params.m_syncs_off << 16) |
		(otf_video_params.m_syncs_off << 8) | otf_video_params.m_syncs_off;

    int adj = 0;
    otf_video_params.m_hfp_pixels += adj;
	otf_video_params.m_hbp_pixels -= adj;

	//otf_video_params.dump();

    uint8_t oldMode = videoMode;
	videoMode = mode;

	TaskHandle_t xHandle = NULL;
	xTaskCreatePinnedToCore(otf_task, "OTF-MODE", 4096, (void*) options,
							OTF_MANAGER_PRIORITY, &xHandle, 1); // Core #1
	while (videoMode == mode);
	return 0; // success
}

bool is_otf_mode() {
	return (di_manager != 0);
}

void otf_print(const char* text) {
	while (*text) {
		di_manager->store_character(*text++);
	}
}

#endif
