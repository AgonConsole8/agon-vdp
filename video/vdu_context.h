#ifndef VDU_CONTEXT_H
#define VDU_CONTEXT_H

#include <memory>
#include <vector>
#include <unordered_map>

#include "context.h"
#include "types.h"
#include "vdu_stream_processor.h"

std::unordered_map<uint8_t, std::shared_ptr<Context>> contexts;	// Storage for our contexts

/*
- select (making a new context if necessary)
- reset
- copy (make a new context copying existing one, and select it)
- delete
- save (as numbered context for later selection) ?
  dependent on exact API design, if we have automatic context creation this could be useful
- push ?
  similar in concept to the saveGState() call in Apple's Core Graphics - save a copy of the current context to a stack
  this would be an alternative way to create a new state without using "select"
- push and select ?
  push current context to stack, and select a numbered context to replace it. this would be similar to calling push and then select, but would omit the creation of a new context that would otherwise just get thrown away
- pop ?
  similar in context to restoreGState() in Core Graphics - restoring to last context
- popAll ?
  effectively just pop until you can't pop no more... you'd be left at the original (top-level) context from the stack
- deleteStack ?
  this would remove the stack history, leaving your current context unaffected
- selectStack ?
  wild and crazy idea to save the whole current stack of graphics contexts...
  having said that, if we support context stacks, this may replace plain "select"

with context stack things, this _might_ mean...
  - "select" and "delete" apply to current stack, not just single context
  - "reset" maybe should reset just the current context ?
  - "copy" should copy the current context, making a new stack
  - "push and select" should maybe select only the top-most context at the given ID?

*/

bool contextExists(uint8_t id) {
	return contexts.find(id) != contexts.end();
}

void resetAllContexts() {
	// Not sure about this
	// when would we call this?
	// a mode change could perform this
	// but maybe it would be more appropriate to reset all processors to a new/default single context?
	for (auto it = contexts.begin(); it != contexts.end(); ++it) {
		auto& id = it->first;
		auto& context = it->second;
		context->reset();
	}
}

void VDUStreamProcessor::vdu_sys_context() {
	auto command = readByte_t(); if (command == -1) return;

	switch (command) {
		case CONTEXT_SELECT: {	// VDU 23, 0, &C8, 0, id
			auto id = readByte_t(); if (id == -1) return;
			selectContext(id);
			debug_log("vdu_sys_context: select %d\n\r", id);
		} break;
		case CONTEXT_RESET: {	// VDU 23, 0, &C8, 1
			context->reset();
			debug_log("vdu_sys_context: reset\n\r");
		} break;
		case CONTEXT_DELETE: {	// VDU 23, 0, &C8, 2, id
			auto id = readByte_t(); if (id == -1) return;
			if (contextExists(id)) {
				contexts.erase(id);
				debug_log("vdu_sys_context: delete %d\n\r", id);
			} else {
				debug_log("vdu_sys_context: delete %d not found\n\r", id);
			}
		} break;
		case CONTEXT_SAVE: {	// VDU 23, 0, &C8, 3, id
			auto id = readByte_t(); if (id == -1) return;
			contexts[id] = context;
			debug_log("vdu_sys_context: save %d\n\r", id);
		} break;
		case CONTEXT_PUSH: {	// VDU 23, 0, &C8, 4
			debug_log("vdu_sys_context: push not yet implemented\n\r");
		} break;
		case CONTEXT_PUSH_AND_SELECT: {	// VDU 23, 0, &C8, 5, id
			debug_log("vdu_sys_context: push_and_select not yet implemented\n\r");
		} break;
		case CONTEXT_POP: {		// VDU 23, 0, &C8, 6
			debug_log("vdu_sys_context: pop not yet implemented\n\r");
		} break;
		case CONTEXT_POPALL: {	// VDU 23, 0, &C8, 7
			debug_log("vdu_sys_context: popall not yet implemented\n\r");
		} break;
		case CONTEXT_DELETESTACK: {	// VDU 23, 0, &C8, 8
			debug_log("vdu_sys_context: deletestack not yet implemented\n\r");
		} break;
	}
}

void VDUStreamProcessor::selectContext(uint8_t id) {
	if (contextExists(id)) {
		debug_log("selectContext: selecting existing context %d\n\r", id);
		context = contexts[id];
		context->activate();
	} else {
		debug_log("selectContext: creating new context %d\n\r", id);
		// create new context
		context = make_shared_psram<Context>(*context);

		contexts[id] = context;
	}
}

void VDUStreamProcessor::resetContext() {
	context->reset();
}

#endif // VDU_CONTEXT_H
