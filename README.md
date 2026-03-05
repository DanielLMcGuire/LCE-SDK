C++ Modding SDK for LCE

Work in progress, commits are (strongly) welcomed

I plan to bind this to a higher level language when complete

> [!WARNING]
>
> Not stable (yet)
>
> Might also be important to know that it doesnt work yet, and is client side rn

## Requirements
### 1. Source Patches:
Client folder contains them

### 2. Create a mod:
Create a main function:

```cpp
#include "../Minecraft.Client/Minecraft.h"

void minecraft_mod(Minecraft* minecraft) {
...
}
```

Create a thread and include ANYTHING from ../Minecraft.Client/, and manipulate anything under the Minecraft class or it's members inside the new thread.

Post init stuff can be done before the thread, but you should read the code to understand it's limitations at this stage.

You will need to use the same compiler to compile the mod as you compiled the fork to compile a library (DLL file), and place it in MINECRAFT_DIR/mods/

Still working on a cleaned up interface

Other modding resources/tools:
https://github.com/NessieHax/mc-arc-util
https://dlce.pages.dev/
