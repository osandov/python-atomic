#include <Python.h>

typedef struct {
	PyObject_HEAD
	long value;
} Integer;

static int Integer_init(Integer *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = {"x", NULL};

	if (!__atomic_is_lock_free(sizeof(self->value), &self->value)) {
		if (PyErr_WarnEx(PyExc_RuntimeWarning,
				 "atomic.Integer is not lock free", 1) < 0)
			return -1;
	}

	self->value = 0;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|l", kwlist,
					 &self->value))
		return -1;

	return 0;
}

static PyObject *Integer_repr(Integer *self)
{
	long value;

	__atomic_load(&self->value, &value, __ATOMIC_SEQ_CST);

	return PyUnicode_FromFormat("atomic.Integer(%ld)", value);
}

static PyObject *Integer_str(Integer *self)
{
	long value;

	__atomic_load(&self->value, &value, __ATOMIC_SEQ_CST);

	return PyUnicode_FromFormat("%ld", value);
}

static PyObject *Integer_get(Integer *self)
{
	long x;

	__atomic_load(&self->value, &x, __ATOMIC_SEQ_CST);

	return PyLong_FromLong(x);
}

static PyObject *Integer_set(Integer *self, PyObject *args)
{
	long value;

	if (!PyArg_ParseTuple(args, "l", &value))
		return NULL;

	__atomic_store(&self->value, &value, __ATOMIC_SEQ_CST);

	Py_RETURN_NONE;
}

static PyObject *Integer_get_and_set(Integer *self, PyObject *args)
{
	long value, ret;

	if (!PyArg_ParseTuple(args, "l", &value))
		return NULL;

	__atomic_exchange(&self->value, &value, &ret, __ATOMIC_SEQ_CST);

	return PyLong_FromLong(ret);
}

static PyObject *Integer_compare_and_set(Integer *self, PyObject *args)
{
	long expect, update, ret;

	if (!PyArg_ParseTuple(args, "ll", &expect, &update))
		return NULL;

	ret = __atomic_compare_exchange(&self->value, &expect, &update, 0,
					__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

	return PyBool_FromLong(ret);
}

static PyObject *Integer_weak_compare_and_set(Integer *self, PyObject *args)
{
	long expect, update, ret;

	if (!PyArg_ParseTuple(args, "ll", &expect, &update))
		return NULL;

	ret = __atomic_compare_exchange(&self->value, &expect, &update, 1,
					__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

	return PyBool_FromLong(ret);
}

#define Integer_GET_AND(name)							\
static PyObject *Integer_get_and_##name(Integer *self, PyObject *args)		\
{										\
	long value, ret;							\
										\
	if (!PyArg_ParseTuple(args, "l", &value))				\
		return NULL;							\
										\
	ret = __atomic_fetch_##name(&self->value, value, __ATOMIC_SEQ_CST);	\
										\
	return PyLong_FromLong(ret);						\
}

#define Integer_AND_GET(name)							\
static PyObject *Integer_##name##_and_get(Integer *self, PyObject *args)	\
{										\
	long value, ret;							\
										\
	if (!PyArg_ParseTuple(args, "l", &value))				\
		return NULL;							\
										\
	ret = __atomic_##name##_fetch(&self->value, value, __ATOMIC_SEQ_CST);	\
										\
	return PyLong_FromLong(ret);						\
}

Integer_GET_AND(add)
Integer_GET_AND(sub)
Integer_GET_AND(and)
Integer_GET_AND(xor)
Integer_GET_AND(or)
Integer_GET_AND(nand)

Integer_AND_GET(add)
Integer_AND_GET(sub)
Integer_AND_GET(and)
Integer_AND_GET(xor)
Integer_AND_GET(or)
Integer_AND_GET(nand)

static PyMethodDef Integer_methods[] = {
	{"get", (PyCFunction)Integer_get, METH_NOARGS,
	 "get() -> int\n\n"
	 "Atomically load and return the value of this integer."},
	{"set", (PyCFunction)Integer_set, METH_VARARGS,
	 "set(x)\n\n"
	 "Atomically store the given value in this integer."},

	{"get_and_set", (PyCFunction)Integer_get_and_set, METH_VARARGS,
	 "get_and_set(x) -> int\n\n"
	 "Atomically store the given value and return the old value."},
	{"compare_and_set", (PyCFunction)Integer_compare_and_set, METH_VARARGS,
	 "compare_and_set(expect, update) -> bool\n\n"
	 "Atomically store the given value if the old value equals the given expected\n"
	 "value, returning whether the actual value equaled the expected value."},
	{"weak_compare_and_set", (PyCFunction)Integer_weak_compare_and_set, METH_VARARGS,
	 "weak_compare_and_set(expect, update) -> bool\n\n"
	 "compare_and_set, but can fail spuriously and does not provide ordering\n"
	 "guarantees."},

	{"get_and_add", (PyCFunction)Integer_get_and_add, METH_VARARGS,
	 "get_and_add(x) -> int\n\n"
	 "Atomically add the given value to this integer and return the previously stored\n"
	 "value."},
	{"get_and_sub", (PyCFunction)Integer_get_and_sub, METH_VARARGS,
	 "get_and_sub(x) -> int\n\n"
	 "Atomically subtract the given value from this integer and return the previously\n"
	 "stored value."},
	{"get_and_and", (PyCFunction)Integer_get_and_and, METH_VARARGS,
	 "get_and_and(x) -> int\n\n"
	 "Atomically bitwise-and the given value with this integer and return the\n"
	 "previously stored value."},
	{"get_and_xor", (PyCFunction)Integer_get_and_xor, METH_VARARGS,
	 "get_and_xor(x) -> int\n\n"
	 "Atomically bitwise-xor the given value with this integer and return the\n"
	 "previously stored value."},
	{"get_and_or", (PyCFunction)Integer_get_and_or, METH_VARARGS,
	 "get_and_or(x) -> int\n\n"
	 "Atomically bitwise-or the given value with this integer and return the\n"
	 "previously stored value."},
	{"get_and_nand", (PyCFunction)Integer_get_and_nand, METH_VARARGS,
	 "get_and_nand(x) -> int\n\n"
	 "Atomically bitwise-nand the given value with this integer and return the\n"
	 "previously stored value."},

	{"add_and_get", (PyCFunction)Integer_add_and_get, METH_VARARGS,
	 "add_and_get(x) -> int\n\n"
	 "Atomically add the given value to this integer and return the resulting value."},
	{"sub_and_get", (PyCFunction)Integer_sub_and_get, METH_VARARGS,
	 "sub_and_get(x) -> int\n\n"
	 "Atomically subtract the given value from this integer and return the resulting\n"
	 "value."},
	{"and_and_get", (PyCFunction)Integer_and_and_get, METH_VARARGS,
	 "and_and_get(x) -> int\n\n"
	 "Atomically bitwise-and the given value with this integer and return the\n"
	 "resulting value."},
	{"xor_and_get", (PyCFunction)Integer_xor_and_get, METH_VARARGS,
	 "xor_and_get(x) -> int\n\n"
	 "Atomically bitwise-xor the given value with this integer and return the\n"
	 "resulting value."},
	{"or_and_get", (PyCFunction)Integer_or_and_get, METH_VARARGS,
	 "or_and_get(x) -> int\n\n"
	 "Atomically bitwise-or the given value to this integer and return the resulting\n"
	 "value."},
	{"nand_and_get", (PyCFunction)Integer_nand_and_get, METH_VARARGS,
	 "nand_and_get(x) -> int\n\n"
	 "Atomically bitwise-nand the given value to this integer and return the\n"
	 "resulting value."},

	{NULL, NULL, 0, NULL}
};

#define ATOMIC_INTEGER_DOCSTRING \
	"atomic.Integer(x=0) -> new atomic integer\n\n" \
	"Integer supporting atomic operations with the range of a C long and\n" \
	"sequentially consistent semantics.\n\n" \
	"Atomic load (get()), store (set()), exchange (get_and_set()),\n" \
	"and compare-and-exchange (compare_and_set()) are supported.\n\n" \
	"The get_and_x methods atomically load, update, and store the result of an\n" \
	"operation. They return the value that was previously stored.\n\n" \
	"The x_and_get methods atomically load, update, and store the result of an\n" \
	"operation. They return the result of the operation."

PyTypeObject Integer_type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"atomic.Integer",		/* tp_name */
	sizeof(Integer),		/* tp_basicsize */
	0,				/* tp_itemsize */
	NULL,				/* tp_dealloc */
	NULL,				/* tp_print */
	NULL,				/* tp_getattr */
	NULL,				/* tp_setattr */
	NULL,				/* tp_reserved */
	(reprfunc)Integer_repr,		/* tp_repr */
	NULL,				/* tp_as_number */
	NULL,				/* tp_as_sequence */
	NULL,				/* tp_as_mapping */
	NULL,				/* tp_hash  */
	NULL,				/* tp_call */
	(reprfunc)Integer_str,		/* tp_str */
	NULL,				/* tp_getattro */
	NULL,				/* tp_setattro */
	NULL,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	ATOMIC_INTEGER_DOCSTRING,	/* tp_doc */
	NULL,				/* tp_traverse */
	NULL,				/* tp_clear */
	NULL,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	NULL,				/* tp_iter */
	NULL,				/* tp_iternext */
	Integer_methods,		/* tp_methods */
	NULL,				/* tp_members */
	NULL,				/* tp_getset */
	NULL,				/* tp_base */
	NULL,				/* tp_dict */
	NULL,				/* tp_descr_get */
	NULL,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	(initproc)Integer_init,		/* tp_init */
};
