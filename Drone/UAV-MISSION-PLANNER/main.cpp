#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <functional>
#include <string>
#include <limits>
#include <thread>
#include <chrono>

using namespace std;

// ============================================================
// WAYPOINT
// ============================================================

struct Waypoint
{
    int number;

    double latitude;
    double longitude;
    double altitude;
    double speed;
};

// ============================================================
// MISSION STATE
// ============================================================

enum class MissionState
{
    Empty,
    Ready,
    Uploaded,
    Running,
    Paused,
    Completed,
    Aborted
};

// ============================================================
// MISSION
// ============================================================

class Mission
{
private:

    vector<Waypoint> waypoints;

    MissionState state = MissionState::Empty;

    size_t currentWaypoint = 0;

public:

    // --------------------------------------------------------
    // ADD WAYPOINT
    // --------------------------------------------------------

    void addWaypoint()
    {
        Waypoint wp;

        wp.number = static_cast<int>(waypoints.size()) + 1;

        cout << "\nLatitude: ";
        cin >> wp.latitude;

        cout << "Longitude: ";
        cin >> wp.longitude;

        cout << "Altitude (m): ";
        cin >> wp.altitude;

        cout << "Speed (m/s): ";
        cin >> wp.speed;

        if (cin.fail())
        {
            cin.clear();

            cin.ignore(
                numeric_limits<streamsize>::max(),
                '\n'
            );

            cout << "\nInvalid waypoint data.\n";
            return;
        }

        waypoints.push_back(wp);

        state = MissionState::Ready;

        cout << "\nWaypoint "
             << wp.number
             << " added.\n";
    }

    // --------------------------------------------------------
    // REMOVE LAST WAYPOINT
    // --------------------------------------------------------

    void removeWaypoint()
    {
        if (waypoints.empty())
        {
            cout << "\nMission has no waypoints.\n";
            return;
        }

        cout << "\nRemoved waypoint "
             << waypoints.back().number
             << ".\n";

        waypoints.pop_back();

        if (waypoints.empty())
            state = MissionState::Empty;
    }

    // --------------------------------------------------------
    // SHOW MISSION
    // --------------------------------------------------------

    void showMission() const
    {
        cout << "\n============================================================\n";
        cout << "                     MISSION PLAN\n";
        cout << "============================================================\n";

        cout << "Status          : "
             << stateToString()
             << '\n';

        cout << "Total Waypoints : "
             << waypoints.size()
             << '\n';

        if (waypoints.empty())
        {
            cout << "\nNo waypoints.\n";
            cout << "============================================================\n";
            return;
        }

        cout << "\n";

        cout << left
             << setw(5)  << "WP"
             << setw(14) << "Latitude"
             << setw(14) << "Longitude"
             << setw(12) << "Altitude"
             << setw(10) << "Speed"
             << '\n';

        cout << "------------------------------------------------------------\n";

        cout << fixed << setprecision(6);

        for (const auto& wp : waypoints)
        {
            cout << left
                 << setw(5)  << wp.number
                 << setw(14) << wp.latitude
                 << setw(14) << wp.longitude
                 << setw(12) << wp.altitude
                 << setw(10) << wp.speed
                 << '\n';
        }

        cout << "============================================================\n";
    }

    // --------------------------------------------------------
    // CLEAR MISSION
    // --------------------------------------------------------

    void clearMission()
    {
        waypoints.clear();

        state = MissionState::Empty;

        currentWaypoint = 0;

        cout << "\nMission cleared.\n";
    }

    // --------------------------------------------------------
    // UPLOAD
    // --------------------------------------------------------

    void upload()
    {
        if (waypoints.empty())
        {
            cout << "\nCannot upload empty mission.\n";
            return;
        }

        state = MissionState::Uploaded;

        currentWaypoint = 0;

        cout << "\nMission uploaded successfully.\n";
        cout << "Waypoints uploaded: "
             << waypoints.size()
             << '\n';
    }

    // --------------------------------------------------------
    // START
    // --------------------------------------------------------

    void start()
    {
        if (waypoints.empty())
        {
            cout << "\nNo mission available.\n";
            return;
        }

        if (state != MissionState::Uploaded &&
            state != MissionState::Paused)
        {
            cout << "\nMission must be uploaded first.\n";
            return;
        }

        if (state == MissionState::Paused)
        {
            resume();
            return;
        }

        state = MissionState::Running;

        currentWaypoint = 0;

        executeMission();
    }

    // --------------------------------------------------------
    // EXECUTE MISSION
    // --------------------------------------------------------

    void executeMission()
    {
        while (currentWaypoint < waypoints.size())
        {
            if (state != MissionState::Running)
                return;

            const auto& wp =
                waypoints[currentWaypoint];

            cout << "\n------------------------------------------------------------\n";

            cout << "Flying to Waypoint "
                 << wp.number
                 << '\n';

            cout << "Latitude  : "
                 << fixed << setprecision(6)
                 << wp.latitude
                 << '\n';

            cout << "Longitude : "
                 << wp.longitude
                 << '\n';

            cout << "Altitude  : "
                 << wp.altitude
                 << " m\n";

            cout << "Speed     : "
                 << wp.speed
                 << " m/s\n";

            cout << "------------------------------------------------------------\n";

            // Simulate UAV flight
            for (int progress = 0;
                 progress <= 100;
                 progress += 20)
            {
                if (state != MissionState::Running)
                    return;

                cout << "\rProgress: "
                     << setw(3)
                     << progress
                     << "%";

                cout.flush();

                this_thread::sleep_for(
                    chrono::milliseconds(300)
                );
            }

            cout << "\nReached Waypoint "
                 << wp.number
                 << ".\n";

            ++currentWaypoint;
        }

        state = MissionState::Completed;

        cout << "\n============================================================\n";
        cout << "MISSION COMPLETED\n";
        cout << "============================================================\n";
    }

    // --------------------------------------------------------
    // PAUSE
    // --------------------------------------------------------

    void pause()
    {
        if (state != MissionState::Running)
        {
            cout << "\nMission is not running.\n";
            return;
        }

        state = MissionState::Paused;

        cout << "\nMission paused at Waypoint ";

        if (currentWaypoint < waypoints.size())
            cout << waypoints[currentWaypoint].number;
        else
            cout << "-";

        cout << ".\n";
    }

    // --------------------------------------------------------
    // RESUME
    // --------------------------------------------------------

    void resume()
    {
        if (state != MissionState::Paused)
        {
            cout << "\nMission is not paused.\n";
            return;
        }

        state = MissionState::Running;

        cout << "\nMission resumed.\n";

        executeMission();
    }

    // --------------------------------------------------------
    // ABORT
    // --------------------------------------------------------

    void abort()
    {
        if (state != MissionState::Running &&
            state != MissionState::Paused)
        {
            cout << "\nNo active mission to abort.\n";
            return;
        }

        state = MissionState::Aborted;

        cout << "\nMISSION ABORTED.\n";
    }

    // --------------------------------------------------------
    // STATUS
    // --------------------------------------------------------

    void status() const
    {
        cout << "\n---------------- MISSION STATUS ----------------\n";

        cout << "State           : "
             << stateToString()
             << '\n';

        cout << "Waypoints       : "
             << waypoints.size()
             << '\n';

        if (currentWaypoint < waypoints.size())
        {
            cout << "Current WP      : "
                 << waypoints[currentWaypoint].number
                 << '\n';
        }
        else
        {
            cout << "Current WP      : -\n";
        }

        cout << "-------------------------------------------------\n";
    }

private:

    string stateToString() const
    {
        switch (state)
        {
            case MissionState::Empty:
                return "EMPTY";

            case MissionState::Ready:
                return "READY";

            case MissionState::Uploaded:
                return "UPLOADED";

            case MissionState::Running:
                return "RUNNING";

            case MissionState::Paused:
                return "PAUSED";

            case MissionState::Completed:
                return "COMPLETED";

            case MissionState::Aborted:
                return "ABORTED";
        }

        return "UNKNOWN";
    }
};

// ============================================================
// DASHBOARD
// ============================================================

class Dashboard
{
public:

    static void clear()
    {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    static void show()
    {
        cout << "\n";
        cout << "============================================================\n";
        cout << "                    UAV MISSION PLANNER\n";
        cout << "============================================================\n";

        cout << "\n[MISSION COMMANDS]\n\n";

        cout << " 1. Add Waypoint\n";
        cout << " 2. Remove Last Waypoint\n";
        cout << " 3. Show Mission\n";
        cout << " 4. Clear Mission\n";
        cout << " 5. Upload Mission\n";
        cout << " 6. Start Mission\n";
        cout << " 7. Pause Mission\n";
        cout << " 8. Resume Mission\n";
        cout << " 9. Abort Mission\n";
        cout << "10. Mission Status\n";
        cout << " 0. Exit\n";

        cout << "\n============================================================\n";
        cout << "Command: ";
    }
};

// ============================================================
// APPLICATION
// ============================================================

class Application
{
private:

    Mission mission;

    unordered_map<int, function<void()>> commands;

public:

    Application()
    {
        commands =
        {
            {1, [&] {
                mission.addWaypoint();
            }},

            {2, [&] {
                mission.removeWaypoint();
            }},

            {3, [&] {
                mission.showMission();
            }},

            {4, [&] {
                mission.clearMission();
            }},

            {5, [&] {
                mission.upload();
            }},

            {6, [&] {
                mission.start();
            }},

            {7, [&] {
                mission.pause();
            }},

            {8, [&] {
                mission.resume();
            }},

            {9, [&] {
                mission.abort();
            }},

            {10, [&] {
                mission.status();
            }}
        };
    }

    void run()
    {
        while (true)
        {
            Dashboard::clear();

            mission.status();

            Dashboard::show();

            int command;

            if (!(cin >> command))
            {
                cin.clear();

                cin.ignore(
                    numeric_limits<streamsize>::max(),
                    '\n'
                );

                continue;
            }

            if (command == 0)
            {
                cout << "\nExiting Mission Planner...\n";
                break;
            }

            auto it =
                commands.find(command);

            if (it != commands.end())
            {
                it->second();
            }
            else
            {
                cout << "\nInvalid command.\n";
            }

            cout << "\nPress ENTER to continue...";

            cin.ignore(
                numeric_limits<streamsize>::max(),
                '\n'
            );

            cin.get();
        }
    }
};

// ============================================================
// MAIN
// ============================================================

int main()
{
    Application app;

    app.run();

    return 0;
}