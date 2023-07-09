# Fire Breathing Dragon
![Screenshot](https://raw.githubusercontent.com/Aleman778/Fire-Breathing-Dragon/main/screenshot.png)

My entry for the GMTK Game Jam 2023. 
The theme for this jam is **Roles Reversed** which means you will be playing a role that you normally don't play.

As the theme suggests, in this game the roles have been reversed and you play as the boss enemy, a red fire breathing dragon with a single goal to defeat the hero.

## Controls
- Move left and right with **W/ D**
- Fly downwards faster with **S**
- Fly upwards with **SPACE**
- Breathe fire with **F** (NOTE: there is a cooldown for this action)

## How to play
The take control of the dragon starting on the left of the screen and naviage to 
your enemey the hero wearing a blue space suit. To defeat your enemy you need to 
be able to dodge enemy attacks and fight back using your fire breathing ability.
Your fire breathing cannot be used too frequently and you cannot move while
attacking so picking the right time to attack is the key to victory.

## Development Journey

I started out writing the game using my own programming language https://github.com/Aleman778/sqrrl together with Raylib as the "engine". My programming language is very close to C/C++ so it works well for game development. But later on when I uploaded my first version I find that windows defender just straight up deletes my .exe because it's contains a virus (when it actually doesn't). And therefore I had to rewrite the code to work with normal C/C++ compiler which was painful but didn't take very long because of the similarities in those languages. But this was worth it since my language didn't have WASM support yet, using https://emscripten.org/. I could make we web build which I unfortunately couldn't get the music to work in.

Also to complement this game I had to draw assets and make sounds and music. The dragon was probably the most time consuming part but I did in 3 iterations. Early on in the first iteration I just created a simple shape, then secondly I made it animate the legs and finally the last hours I added the tail and wings.

Some sounds were made using this https://sfxr.me/  very awesome tool and I slapped together some music using https://www.beepbox.co/#9n31s1k0l00e08t3ea7g0hj07r1i0o432T3v4ughf0q0x10j51d19SXi...

Also my mouth was used to make sounds for the dragon and enemy getting hurt. These sounds were heavily altered using https://www.audacityteam.org/ .
