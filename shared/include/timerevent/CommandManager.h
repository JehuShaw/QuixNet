/* 
 * File:   CommandManager.h
 * Author: Jehu Shaw
 *
 * Created on 2010_11_12, 9:28
 */

#ifndef COMMANDMANAGER_H
#define	COMMANDMANAGER_H

#include <string>
#include <vector>
#include <algorithm>
#include "SimpleEvent.h"
#include "Singleton.h"

namespace evt
{
	enum SHARED_DLL_DECL CommandResult {
		COMMAND_RESULT_INVALID,
		COMMAND_RESULT_FAIL,
		COMMAND_RESULT_SUCCESS,
		COMMAND_RESULT_EXIT,
	};

	class CommandArgument : public ArgumentBase {
	public:
		CommandArgument(std::string params) : m_params(params) {}

		const std::string& GetParams() { return m_params; }
	private:

		std::string m_params;
	};

	class CommandEvent : public MethodRIP1Base {
	public:
		CommandEvent(const util::CAutoPointer<MethodRIP1Base>& method, const std::string& spec)
			: m_method(method), m_spec(spec) {}

        int Invoke(const util::CWeakPointer<ArgumentBase>& arg) {
            if(m_method.IsInvalid()) {
                return 0;
            }
            return m_method->Invoke(arg);
        }
		const std::string& GetSpec() {
			return m_spec;
		}
	protected:
        util::CAutoPointer<MethodRIP1Base> m_method;
		std::string m_spec;
	};

	typedef struct CommandInfo {
		std::string cmd;
		std::string spec;
	}CommandInfo;

	typedef std::vector<CommandInfo> SHOW_COMMAND_LIST_T;

    class SHARED_DLL_DECL CommandManager 
		: private SimpleEvent<std::string>
		, public util::Singleton<CommandManager>
	{
    public:

		CommandManager() {}

		~CommandManager() {}

        bool AddCommand(std::string cmd, util::CAutoPointer<MethodRIP1Base> method, std::string specification) {
			std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower); 
			util::CAutoPointer<CommandEvent> commandEvent(new CommandEvent(method, specification));
			return AddEventListener(cmd, commandEvent);
		}

        CommandResult DispatchCommand(std::string cmd, util::CWeakPointer<ArgumentBase> arg) {
			std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower); 
			return (CommandResult)DispatchEvent(cmd, arg);
		}

		bool HasCommand(std::string cmd) {
			std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower); 
			return HasEventListener(cmd);
		}

		void RemoveCommand(std::string cmd) {
			std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower); 
			RemoveEventListener(cmd);
		}

        void ClearCommand() {
			Clear();
		}

		void ShowCommand(SHOW_COMMAND_LIST_T& outList);
	};

} // end namespace evt

#endif	/* COMMANDMANAGER_H */

