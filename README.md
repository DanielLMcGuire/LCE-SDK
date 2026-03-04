Modding SDK for LCE

Work in progress, commits are (strongly) welcomed

> [!WARNING]
>
> Not stable (yet)

## Requirements
### 1. Source Patches:
Minecraft.Client contains them

### 2. Create a mod:
Create a main function:

```cpp
#include "../Minecraft.Client/Minecraft.h"

void minecraft_mod(Minecraft* minecraft) {
...
}
```

Create a thread and include ANYTHING from ../Minecraft/Client, and manipulate anything under the Minecraft class or it's members.

You will need to use the same compiler to compile the mod as you compiled the fork.

Still working on a cleaned up interface

Other modding resources/tools:
https://github.com/NessieHax/mc-arc-util
https://dlce.pages.dev/
