### Simple Python2 interface to Godot (CPython embedded)

Build a single static library with embedded CPython.

Important pieces of implememtation are:

1. C-Api interface to _Godot_ filesystem: [_py_file.cpp](CPython/Godot/_py_file.cpp)
2. Interface to _Godot_ functions: [py_godot.h](pylib/godot/py_godot.h)
3. Extended bytecode caching

Example of usage:

```
class GodotInterface:
	"""Godot interface to the main application
	"""
	def __init__( self, instance_id ):
		self.instance_id = instance_id

	def gd_init( self ):
		self.app = MyPythonApp( )

	def gd_tick( self, delta ):
		self.app.game_tick( delta )

	def gd_event( self, event ):
		self.app.game_event( event )

	def gd_term( self ):
		self.app.game_term( )

def _gd_build(instance_id):
	return GodotInterface( instance_id )
```

__Note:__

  * ```_gd_build``` is a name that can be customized in _CPythonInstance_ properties.
  * ```instance_id``` is Godot's instance id of _CPythonInstance_ node (later it is necessery for drawing), eg:

```
	void _blit(int instance_id, const GdSurface &source, const Point2 &dest, const Rect2 &area) {
		if (Object *parent = ObjectDB::get_instance(instance_id)) {
			if (CanvasItem *canvas = Object::cast_to<CanvasItem>(parent)) {
			} else {
				WARN_PRINT("Not an CanvasItem");
			}
		}
	}
```

  * interface to Godot consist of four methods: ```gd_init( instacne_id )```, ```gd_tick( delta )```, ```gd_event( event )```, ```gd_term( )```

  * alternative way of caching of bytecode (similar to python3) is enabled when env. variable _PYTHONPYCACHEPREFIX_ point to a valid directory.
    All bytecodes is keeping in given folder (coming from every source used, also from zip archives) in flat format, eg:
```
-rw-r--r--  1 piecuchp  staff  18704 Dec 28 16:54 pycache/pylib.zip.sre_parse.pyo
-rw-r--r--  1 piecuchp  staff   2641 Dec 28 16:54 pycache/pylib.zip.stat.pyo
-rw-r--r--  1 piecuchp  staff    228 Dec 28 16:54 pycache/pylib.zip.struct.pyo
-rw-r--r--  1 piecuchp  staff   2440 Dec 28 16:54 pycache/pylib.zip.types.pyo
-rw-r--r--  1 piecuchp  staff  12046 Dec 28 16:54 pycache/pylib.zip.warnings.pyo
-rw-r--r--  1 piecuchp  staff  13487 Dec 28 16:54 pycache/pylib.zip.weakref.pyo
-rw-r--r--  1 piecuchp  staff   7467 Dec 25 22:33 pycache/scripts.control.pyo
-rw-r--r--  1 piecuchp  staff   2088 Dec 25 22:33 pycache/scripts.controlview.pyo
-rw-r--r--  1 piecuchp  staff    119 Dec 25 22:33 pycache/scripts.engine.__init__.pyo
-rw-r--r--  1 piecuchp  staff   3941 Dec 30 20:49 pycache/scripts.engine.app.pyo
```

---

From original ReadMe (https://github.com/albertz/python-embedded):

> Build a single static library with Python and PyCrypto.
> This passes `test_crypto()` in [binstruct](https://github.com/albertz/binstruct/).
>
> I tried a sample iOS binary where I linked against `libpython.a` and copied the `pylib` directory into Resources (following not-included symlinks).
>
> This is the sample code:
>
>    Py_SetProgramName((char*)[[[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/"] UTF8String]);
>    Py_Initialize();
>    PyRun_SimpleString("print 'Hello world!'");
>
> And it works. :)

## Reference:

1. https://github.com/albertz/python-embedded
2. https://github.com/RomkoSI/Blender3D/blob/d8ee882db43daeb4859348e94a66abd0353150aa/source/blender/python/generic/bgl.c#L1693
3. http://python3porting.com/cextensions.html
4. https://realpython.com/build-python-c-extension-module/
5. https://groups.google.com/g/cython-users/c/G3O6YM6YgY4
6. https://stackoverflow.com/questions/39250524/programmatically-define-a-package-structure-in-embedded-python-3
7. https://www.oreilly.com/library/view/python-cookbook/0596001673/ch16s06.html
8. https://stackabuse.com/enhancing-python-with-custom-c-extensions
9. https://github.com/pasimako/embedPython
10. https://stackoverflow.com/questions/42521830/call-a-python-function-from-c-using-pybind11

*Android:*
1. https://bugs.python.org/issue23496

*c++/Qt:*
1. https://ubuverse.com/embedding-the-python-interpreter-in-a-qt-application/
2. http://mateusz.loskot.net/post/2011/12/01/python-sys-stdout-redirection-in-cpp/
3. https://codereview.stackexchange.com/questions/92266/sending-a-c-array-to-python-numpy-and-back/92353#92353
4. https://ubuverse.com/embedding-the-python-interpreter-in-a-qt-application/
5. https://docs.scipy.org/doc//numpy-1.15.0/reference/c-api.html
6. https://docs.python.org/3/extending/embedding.html
7. https://github.com/pasimako/embedPython
