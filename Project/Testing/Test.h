#pragma once

#include "../Bridge/Scheduler.h"

namespace Test {

	int Window() {
        string TotalCommand = "";

        string CurrentCommand = "";

        cout << "* Type 'run' on new line after typing script and then return to run. *" << endl;

        while (1) {
            cout << count(TotalCommand.begin(), TotalCommand.end(), '\n') + 1 << ". ";
            getline(cin, CurrentCommand);

            if (CurrentCommand == "run") {
                Scheduler::Lua::RunScript(TotalCommand);

                TotalCommand = "";
            }
            else {
                TotalCommand += CurrentCommand + "\n";
            }
        }

        return 0;
	}

    int Start() {
        thread t(Window);
        t.detach();

        return 0;
    }
}