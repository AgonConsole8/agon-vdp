#ifndef VDU_CONTEXT_H
#define VDU_CONTEXT_H

#include <memory>
#include <vector>
#include <unordered_map>

#include "context.h"
#include "types.h"
#include "vdu_stream_processor.h"

void VDUStreamProcessor::vdu_sys_context() {
	auto command = readByte_t(); if (command == -1) return;

	switch (command) {
		case CONTEXT_SELECT: {	// VDU 23, 0, &C8, 0, id
			auto id = readByte_t(); if (id == -1) return;
			// select context (including stack) with given ID
			// this will duplicate the current stack if the ID does not exist
			selectContext(id);
			sendModeInformation();
			debug_log("vdu_sys_context: select %d\n\r", id);
		} break;
		case CONTEXT_DELETE: {	// VDU 23, 0, &C8, 1, id
			auto id = readByte_t(); if (id == -1) return;
			// removes stack with given ID from storage
			if (id != contextId && contextExists(id)) {
				contextStacks.erase(id);
				debug_log("vdu_sys_context: delete %d\n\r", id);
			} else {
				debug_log("vdu_sys_context: delete %d not found, or is active context\n\r", id);
			}
		} break;
		case CONTEXT_RESET: {	// VDU 23, 0, &C8, 2, flags
			auto flags = readByte_t(); if (flags == -1) return;
			if (resetContext(flags)) {
				sendModeInformation();
			}
			// Context reset specifically for the current context
			// stack is left intact
			debug_log("vdu_sys_context: reset\n\r");
		} break;
		case CONTEXT_SAVE: {	// VDU 23, 0, &C8, 3
			// copy stack and push to stack - effectively saving the current context
			saveContext();
		} break;
		case CONTEXT_RESTORE: {	// VDU 23, 0, &C8, 4
			// pop stack and restore context
			restoreContext();
			sendModeInformation();
		} break;
		case CONTEXT_SAVE_AND_SELECT: {	// VDU 23, 0, &C8, 5, id
			auto id = readByte_t(); if (id == -1) return;
			saveAndSelectContext(id);
			sendModeInformation();
		} break;
		case CONTEXT_RESTORE_ALL: {		// VDU 23, 0, &C8, 6
			restoreAllContexts();
			sendModeInformation();
		} break;
		case CONTEXT_CLEAR_STACK: {	// VDU 23, 0, &C8, 7
			clearContextStack();
		} break;
		case CONTEXT_DEBUG: {	// VDU 23, 0, &C8, &80
			debug_log("vdu_sys_context: selected context stack ID %d\n\r", contextId);
			debug_log("vdu_sys_context: current stack size %d\n\r", contextStack->size());
			debug_log("vdu_sys_context: available contexts %d\n\r", contextStacks.size());
			for (auto it = contextStacks.begin(); it != contextStacks.end(); ++it) {
				debug_log("vdu_sys_context: context id %d, stack size %d\n\r", it->first, it->second->size());
			}
		} break;
	}
}

void VDUStreamProcessor::selectContext(uint8_t id) {
	if (contextExists(id)) {
		debug_log("selectContext: selecting existing context %d\n\r", id);
		contextStack = contextStacks[id];
		context = contextStack->back();
		context->activate();
	} else {
		debug_log("selectContext: creating new context %d\n\r", id);
		// copy current stack
		auto newStack = make_shared_psram<ContextVector>();
		for (auto it = contextStack->begin(); it != contextStack->end(); ++it) {
			newStack->push_back(make_shared_psram<Context>(*it->get()));
		}
		contextStacks[id] = newStack;
		contextStack = contextStacks[id];
		context = contextStack->back();
	}
	contextId = id;
}

bool VDUStreamProcessor::resetContext(uint8_t flags) {
	// if flags are all clear, then do a "mode" style reset
	if (flags == 0) {
		context->reset();
		return true;
	}

	// otherwise we can reset specific things
	bool sendModeData = false;

	if (flags & CONTEXT_RESET_GPAINT) {	// graphics painting options
		context->resetGraphicsPainting();
		context->resetGraphicsOptions();
	}
	if (flags & CONTEXT_RESET_GPOS) {	// graphics positioning incl graphics viewport
		context->setLogicalCoords(true);
		context->resetGraphicsPositioning();
	}
	if (flags & CONTEXT_RESET_TPAINT) {	// text painting options
		context->resetTextPainting();
	}
	if (flags & CONTEXT_RESET_FONTS) {	// fonts
		context->resetFonts();
		sendModeData = true;
	}
	if (flags & CONTEXT_RESET_TBEHAVIOUR) {	// text cursor behaviour
		context->setCursorBehaviour(0, 0);
		sendModeData = true;
	}
	if (flags & CONTEXT_RESET_TCURSOR) {	// text cursor incl text viewport
		context->resetTextCursor();
		sendModeData = true;
	}
	if (flags & CONTEXT_RESET_CHAR2BITMAP) {	// char-to-bitmap mappings
		context->resetCharToBitmap();
	}

	return sendModeData;
}

void VDUStreamProcessor::saveContext() {
	debug_log("saveContext: saving context\n\r");
	// create a new context and push it to the stack
	auto newContext = make_shared_psram<Context>(*(context.get()));
	contextStack->push_back(newContext);
	context = newContext;
}

void VDUStreamProcessor::restoreContext() {
	if (contextStack->size() > 1) {
		debug_log("restoreContext: restoring context\n\r");
		contextStack->pop_back();
		context = contextStack->back();
		context->activate();
	} else {
		debug_log("restoreContext: no context to restore\n\r");
	}
}

void VDUStreamProcessor::saveAndSelectContext(uint8_t id) {
	// grab a copy of the top-most context at id
	if (contextExists(id)) {
		// grab a copy of the top-most context at id
		debug_log("saveAndSelectContext: selecting existing context %d\n\r", id);
		context = make_shared_psram<Context>(*contextStacks[id]->back());
		contextStack->push_back(context);
		context->activate();
	} else {
		debug_log("saveAndSelectContext: context %d not found\n\r", id);
		saveContext();
	}
}

void VDUStreamProcessor::restoreAllContexts() {
	// restore to first context in stack
	if (contextStack->size() > 1) {
		debug_log("restoreAllContexts: restoring all contexts\n\r");
		context = contextStack->front();
		contextStack->clear();
		contextStack->push_back(context);
		context->activate();
	} else {
		debug_log("restoreAllContexts: no contexts to restore\n\r");
	}
}

void VDUStreamProcessor::clearContextStack() {
	debug_log("clearContextStack: clearing all contexts\n\r");
	contextStack->clear();
	contextStack->push_back(context);
}

// Context reset, performed when changing screen modes
//
void VDUStreamProcessor::resetAllContexts() {
	debug_log("resetAllContexts: resetting all contexts\n\r");
	selectContext(0);
	clearContextStack();
	contextStacks.clear();
	contextStacks[0] = contextStack;
	// perform a "mode" style reset
	resetContext(0);
}

#endif // VDU_CONTEXT_H
