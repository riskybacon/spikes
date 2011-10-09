Uses a hacked up version Nate Robbin's OBJ reader that is distributed with
his gltutors code.

Nate's code is very thoughtful and well done. However, it uses
glBegin() / glEnd(), which is deprecated OpenGL functionality.
Instead, buffer objects and glDrawArrays needs to be used.

I've hacked in some functionality to build std::vectors of the
required data, and the caller uses this data, as opposed to the
model reader issuing the commands.

This gives some increased flexibility and less OpenGL coupling.

The addition to the OBJ reader was a quick-n-dirty change and does
not match the thoughtfulness and carefulness  of Nate's original code.
