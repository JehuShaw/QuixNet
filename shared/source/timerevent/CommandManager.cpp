#include "CommandManager.h"

namespace evt {

	void CommandManager::ShowCommand(SHOW_COMMAND_LIST_T& outList) {

		outList.resize(eventmap.size());

		thd::CScopedReadLock scopedReadLock(rwTicket);
		event_map_t::iterator it = eventmap.begin();
		for(int i = 0;eventmap.end() != it; ++it,++i) {

			util::CAutoPointer<CommandEvent> commandEvent(it->second);
			if(commandEvent.IsInvalid()) {
				continue;
			}
			CommandInfo& commandInfo = outList[i];
			commandInfo.cmd = it->first;
			commandInfo.spec = commandEvent->GetSpec();
		}
	}

} // end namespace evt

