# MapleLeaf

**MapleLeaf** is an independently-developed ECS game engine based on C23 standard and SDL3. Pursuing **Api-friendliness** and **Efficiency**.

***NOTE:*** *The engine is still in early unstable development. Not all functions are guaranteed to operate properly.* ***Also***, *due to it's my personal work and the work is just for myself learning and showing(at least for a while), I don't guarantee the project progress.*

## Effect

TODO: Waiting for its further improvement...

## About the assets

For convenience, I decide to keep full necessary assets in the folder `assets`, whether made by myself or others. So, I put the READMEs into the sub-folders to demonstrate the related important things.

Check the [catalogue](assets/README.md)

## Third-party notes

Third party dependencies share the common `thirdparty` folder.

Considering the size of the codebase, only sub-folders of the first layer (third-party names, as the sub titles below) are kept in the version control system. So it's necessary to demonstrate all the stuff you should prepare before build:

### sdl

Official `SDL3` header files are organized like:

- SDL.h
- SDL3/
- - various sub-headers...

Now you should fetch these of [version 3.2.20](https://github.com/libsdl-org/SDL/releases/tag/release-3.2.20), and **put SDL.h into SDL3 folder (if non-existent, create)**.

As for other extra SDL series library, such as SDL_ttf.h, etc, the same thing SDL.h above.

### stb

As for [stb](https://github.com/nothings/stb) series **single-header** files, simply fetch and put them into `stb` folder.

Now the project uses:

- stb_ds.h

## Detail notes

**About "bilingual" comments in code:**

Mostly I use English, but sometimes also use Chinese. Just for convenience. But as the project progresses and my English level improves, the English proportion will be continuously increased.

**About the "proportion" of MYSELF doing:**

I am responsible for **All DESIGN, CODING, and DOCUMENT EDITING work of the engine**; AI Agent handles bug fixes found during testing, hidden danger audits, and proposed improvements. *In my opinion, self-dominated project should always be controlled by myself so that the orientation embodies my ideas. But the bug fixing makes me in state of moil and the expectation after fixing is a relatively definite result, so I use the efficient Agent to help me make it.*
## Build

TODO: It's currently somewhat troublesome to interpret the build conditions and preparations. I'm still summarizing and thinking...

## Progress

My brief dev logs here:

- *from early 07/2026:* SDL3 learning, this ECS architecture designing
- *mid 08/2026:* Code implement starts
- *early 09/2026:* The first working demo, first ui showing. 20+ commits
- TODO...
