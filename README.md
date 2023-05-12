


https://github.com/Zydak/Gravity-Simulation/assets/104147005/e08665fc-827f-4814-80e5-deec2f762e29



# Installation

### Download
	git clone --recursive https://github.com/Zydak/Gravity-Simulation.git
	cd Gravity-Simulation 
	git switch Windows

### Vulkan

Go to https://vulkan.lunarg.com/ and install vulkan SDK

Open the project in Visual Studio, go to project properties -> Linker -> Input -> General -> Additional Library Directories -> Enter the Lib folder path which is in the vulkan SDK directory

## Windows  Bugs
I developed this thing on linux and made windows version only beacuse the app is cross platform. But there are couple of bugs on windows version.

- Slow Performance. For some unknown to me reason windows version runs basically 2 times slower than linux one, maybe this has something to do with way that Visual Studio is optimizing release mode, no idea. On Linux I can easly achieve 10,000 speed. On the other hand on Windows I can barely go past 3,000. I hope that's the case only on my machine.
- When you drag your window Windows will pause the entire app execution (at least on win11) and because of the way app works you'll get massive lag. So better drag your window while paused.

