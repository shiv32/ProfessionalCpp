#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <functional>
#include <limits>
#include <cmath>
#include <cstdlib>

using namespace std;

// ============================================================
// TELEMETRY
// ============================================================

struct Telemetry
{
    double latitude = 12.9716;
    double longitude = 77.5946;

    double altitude = 0.0;
    double speed = 0.0;
    double heading = 0.0;

    double battery = 100.0;
    double pitch = 0.0;
    double roll = 0.0;
    double yaw = 0.0;

    bool armed = false;
    bool flying = false;
    bool gpsLock = true;
    bool rcConnected = true;
};

// ============================================================
// UAV CONTROLLER
// ============================================================

class UAVController
{
private:
    Telemetry telemetry;

public:
    const Telemetry &getTelemetry() const
    {
        return telemetry;
    }

    void arm()
    {
        if (!telemetry.gpsLock)
        {
            cout << "Cannot arm: GPS lock unavailable.\n";
            return;
        }

        if (!telemetry.rcConnected)
        {
            cout << "Cannot arm: RC disconnected.\n";
            return;
        }

        if (telemetry.armed)
        {
            cout << "UAV is already armed.\n";
            return;
        }

        telemetry.armed = true;
    }

    void disarm()
    {
        if (telemetry.flying)
        {
            cout << "Cannot disarm while flying.\n";
            return;
        }

        telemetry.armed = false;
    }

    void takeoff()
    {
        if (!telemetry.armed)
        {
            cout << "Cannot takeoff: UAV is not armed.\n";
            return;
        }

        if (telemetry.flying)
        {
            cout << "UAV is already flying.\n";
            return;
        }

        double altitude;

        cout << "Enter takeoff altitude (m): ";

        if (!(cin >> altitude))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Invalid altitude.\n";
            return;
        }

        if (altitude <= 0)
        {
            cout << "Invalid altitude.\n";
            return;
        }

        telemetry.flying = true;
        telemetry.altitude = altitude;
        telemetry.speed = 5.0;
    }

    void land()
    {
        if (!telemetry.flying)
        {
            cout << "UAV is already on ground.\n";
            return;
        }

        telemetry.altitude = 0.0;
        telemetry.speed = 0.0;

        telemetry.pitch = 0.0;
        telemetry.roll = 0.0;

        telemetry.flying = false;
    }

    void changeAltitude()
    {
        if (!telemetry.flying)
        {
            cout << "UAV is not flying.\n";
            return;
        }

        double altitude;

        cout << "Enter new altitude (m): ";

        if (!(cin >> altitude))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Invalid altitude.\n";
            return;
        }

        if (altitude < 0)
        {
            cout << "Invalid altitude.\n";
            return;
        }

        telemetry.altitude = altitude;
    }

    void setFlightData()
    {
        if (!telemetry.flying)
        {
            cout << "UAV is not flying.\n";
            return;
        }

        double speed;
        double heading;

        cout << "Enter speed (m/s): ";

        if (!(cin >> speed))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Invalid speed.\n";
            return;
        }

        cout << "Enter heading (degrees): ";

        if (!(cin >> heading))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Invalid heading.\n";
            return;
        }

        if (speed < 0)
        {
            cout << "Invalid speed.\n";
            return;
        }

        telemetry.speed = speed;

        telemetry.heading = fmod(heading, 360.0);

        if (telemetry.heading < 0)
            telemetry.heading += 360.0;

        telemetry.yaw = telemetry.heading;
    }

    void updateGPS()
    {
        if (!telemetry.gpsLock)
        {
            cout << "GPS lock unavailable.\n";
            return;
        }

        telemetry.latitude += 0.0001;
        telemetry.longitude += 0.0001;
    }

    void toggleGPS()
    {
        telemetry.gpsLock = !telemetry.gpsLock;
    }

    void toggleRC()
    {
        telemetry.rcConnected = !telemetry.rcConnected;
    }

    void simulateBattery()
    {
        if (!telemetry.flying)
        {
            cout << "UAV is not flying.\n";
            return;
        }

        telemetry.battery -= 2.0;

        if (telemetry.battery < 0)
            telemetry.battery = 0;
    }
};

// ============================================================
// DASHBOARD
// ============================================================

class Dashboard
{
public:
    static void clearScreen()
    {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    static void show(const Telemetry &t)
    {
        cout << fixed << setprecision(2);

        cout << "============================================================\n";
        cout << "                       UAV DASHBOARD\n";
        cout << "============================================================\n";

        cout << "\n[FLIGHT STATUS]\n";

        cout << " Armed       : "
             << (t.armed ? "YES" : "NO") << '\n';

        cout << " Flying      : "
             << (t.flying ? "YES" : "NO") << '\n';

        cout << " GPS Lock    : "
             << (t.gpsLock ? "YES" : "NO") << '\n';

        cout << " RC          : "
             << (t.rcConnected ? "CONNECTED" : "DISCONNECTED")
             << '\n';

        cout << "\n[POSITION]\n";

        cout << " Latitude    : " << t.latitude << '\n';
        cout << " Longitude   : " << t.longitude << '\n';
        cout << " Altitude    : " << t.altitude << " m\n";

        cout << "\n[FLIGHT DATA]\n";

        cout << " Speed       : " << t.speed << " m/s\n";
        cout << " Heading     : " << t.heading << " deg\n";
        cout << " Pitch       : " << t.pitch << " deg\n";
        cout << " Roll        : " << t.roll << " deg\n";
        cout << " Yaw         : " << t.yaw << " deg\n";

        cout << "\n[SYSTEM]\n";

        cout << " Battery     : " << t.battery << " %\n";

        cout << "\n============================================================\n";
    }

    static void showMenu()
    {
        cout << "\n";
        cout << "[COMMANDS]\n";
        cout << "1. Arm\n";
        cout << "2. Disarm\n";
        cout << "3. Takeoff\n";
        cout << "4. Land\n";
        cout << "5. Change Altitude\n";
        cout << "6. Set Speed / Heading\n";
        cout << "7. Update GPS\n";
        cout << "8. Toggle GPS\n";
        cout << "9. Toggle RC\n";
        cout << "10. Battery Usage\n";
        cout << "0. Exit\n";

        cout << "\nCommand: ";
    }
};

// ============================================================
// APPLICATION
// ============================================================

class UAVApplication
{
private:
    UAVController uav;

    unordered_map<int, function<void()>> commands;

public:
    UAVApplication()
    {
        commands =
            {   {1, [&]{ uav.arm(); }},
                {2, [&]{ uav.disarm(); }},
                {3, [&]{ uav.takeoff(); }},
                {4, [&]{ uav.land(); }},
                {5, [&]{ uav.changeAltitude(); }},
                {6, [&]{ uav.setFlightData(); }},
                {7, [&]{ uav.updateGPS(); }},
                {8, [&]{ uav.toggleGPS(); }},
                {9, [&]{ uav.toggleRC(); }},
                {10, [&]{ uav.simulateBattery(); }}};
    }

    void run()
    {
        while (true)
        {
            // Show current dashboard
            Dashboard::clearScreen();

            Dashboard::show(uav.getTelemetry());

            // Show commands
            Dashboard::showMenu();

            int choice;

            if (!(cin >> choice))
            {
                cin.clear();

                cin.ignore(numeric_limits<streamsize>::max(),'\n');

                continue;
            }

            // Exit
            if (choice == 0)
                break;

            // Find command
            auto command = commands.find(choice);

            if (command != commands.end())
            {
                // Execute command
                command->second();
            }
            else
            {
                cout << "\nInvalid command.\n";
            }

            // Small pause so user can see command result
            cout << "\nPress ENTER to continue...";

            cin.ignore(numeric_limits<streamsize>::max(),'\n');

            cin.get();
        }
    }
};

// ============================================================
// MAIN
// ============================================================

int main()
{
    UAVApplication app;

    app.run();

    return 0;
}