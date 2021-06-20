### Simple Python2 interface to Godot (CPython embedded)

Build a single static library with Python (https://github.com/albertz/python-embedded).

This passes `test_crypto()` in [binstruct](https://github.com/albertz/binstruct/).

---

I tried a sample iOS binary where I linked against `libpython.a` and copied the `pylib` directory into Resources (following not-included symlinks).

This is the sample code:

    Py_SetProgramName((char*)[[[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/"] UTF8String]);
    Py_Initialize();
    PyRun_SimpleString("print 'Hello world!'");

And it works. :)


## Reference:

1. https://github.com/albertz/python-embedded
2. https://github.com/RomkoSI/Blender3D/blob/d8ee882db43daeb4859348e94a66abd0353150aa/source/blender/python/generic/bgl.c#L1693
3. http://python3porting.com/cextensions.html
4. https://realpython.com/build-python-c-extension-module/
5. https://groups.google.com/g/cython-users/c/G3O6YM6YgY4

*c++/Qt:*
1. https://ubuverse.com/embedding-the-python-interpreter-in-a-qt-application/
2. http://mateusz.loskot.net/post/2011/12/01/python-sys-stdout-redirection-in-cpp/
3. https://codereview.stackexchange.com/questions/92266/sending-a-c-array-to-python-numpy-and-back/92353#92353
4. https://ubuverse.com/embedding-the-python-interpreter-in-a-qt-application/
5. https://docs.scipy.org/doc//numpy-1.15.0/reference/c-api.html
6. https://docs.python.org/3/extending/embedding.html
7. https://github.com/pasimako/embedPython
