C++ Modding SDK for LCE

> [!NOTE]
> **This project is delayed due for many reasons. It is not off the table for me unless I see there is another alternative.**
>
> Commits are still welcomed

I plan to bind this to a higher level language when complete

> [!WARNING]
>
> Not stable (yet)
>
> Might also be important to know that it doesnt work yet, and is client side rn

## Requirements
### 1. Source Patches:
Client folder contains them

Back up your source code first!! (or just don't delete minecraft.rar)

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

Other resources/tools:
https://dlce.pages.dev/

I have no legal affiliation with Microsoft, thier trademarks, thier affiliates trademarks, nor with any other companies intellectual property.
