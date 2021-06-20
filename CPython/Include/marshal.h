
/* Interface for marshal.c */

#ifndef Py_MARSHAL_H
#define Py_MARSHAL_H
#ifdef __cplusplus
extern "C" {
#endif

#define Py_MARSHAL_VERSION 2

PyAPI_FUNC(void) PyMarshal_WriteLongToFile(long, PYFILE *, int);
PyAPI_FUNC(void) PyMarshal_WriteObjectToFile(PyObject *, PYFILE *, int);
PyAPI_FUNC(PyObject *) PyMarshal_WriteObjectToString(PyObject *, int);

PyAPI_FUNC(long) PyMarshal_ReadLongFromFile(PYFILE *);
PyAPI_FUNC(int) PyMarshal_ReadShortFromFile(PYFILE *);
PyAPI_FUNC(PyObject *) PyMarshal_ReadObjectFromFile(PYFILE *);
PyAPI_FUNC(PyObject *) PyMarshal_ReadLastObjectFromFile(PYFILE *);
PyAPI_FUNC(PyObject *) PyMarshal_ReadObjectFromString(const char *, Py_ssize_t);

#ifdef __cplusplus
}
#endif
#endif /* !Py_MARSHAL_H */
