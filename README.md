# EchoBreak
## We are not responsible for your actions!
## For educational purposes only!

`/EchoBreak/source/EchoBreakServer.cpp`
### Assemble
- Linux: `g++ EchoBreakServer.cpp -o server -pthread`
- Mac
  - `c++ -std=c++11 -o server EchoBreakServer.cpp`
  - `g++-13 -std=c++17 -pthread -o server EchoBreakServer.cpp`
- Don't use Windows

Run `installer.py` to populate `eb.net` on every computer to `/bin/eb.net` directory and run it.
`eb.net` is compiled `source/Worker.cpp`
