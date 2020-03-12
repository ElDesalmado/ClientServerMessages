Current stage: In developement.

Version: 0.1.1

### Installation:
On Windows, using Powershell

```
$build = mkdir *path_to_build_folder*
cmake -S ./ -B $build -A x64 -DQt5_DIR="*path_to_qt*/Qt/*version*/*compiler*/lib/cmake/Qt5"
cmake --build $build --config Release
```

### Description:

This is a small example to show how to establish a TCP connection between a Server and a Client and to send messages.
It also utilizes multithreading for the sake of example. 
Application has 3+ threads: 
- main thread on which runs the GUI;
- ServerMessageLoop, which sends counting messages to the message Log;
- Client connections' threads;

Run *Server.exe* to see the example after building. You can enter and send a message to the loop by pressing Enter or pushing the *Send* button.

In current Version the client part is not yet implemented.  
