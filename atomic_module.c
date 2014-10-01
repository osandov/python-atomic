#include <Python.h>

extern PyTypeObject Integer_type;

static struct PyModuleDef atomicmodule = {
	PyModuleDef_HEAD_INIT,
	"atomic",
	"Module providing types supporting atomic operations.",
	-1,
};

PyMODINIT_FUNC PyInit_atomic(void)
{
	PyObject *m;

	Integer_type.tp_new = PyType_GenericNew;
	if (PyType_Ready(&Integer_type) < 0)
		return NULL;

	m = PyModule_Create(&atomicmodule);
	if (m == NULL)
		return NULL;

	Py_INCREF(&Integer_type);
	PyModule_AddObject(m, "Integer", (PyObject *)&Integer_type);

	return m;
}
