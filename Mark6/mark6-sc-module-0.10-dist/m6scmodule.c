/*
 * $Id: m6scmodule.c 1732 2014-01-05 20:04:08Z gbc $
 *
 * Python module for access to scan check from within c-plane
 */

#include <Python.h>
#include "m6scmodule.h"

/*
 * Our errors will all of this type.
 */
static PyObject *M6scError;

/*
 * Wrapper around the underlying C checker
 * In this case, however, we're only checking one file.
 */
static PyObject *m6sc_check_file(PyObject *self, PyObject *args)
{
    const char *file;
    int scs, verb;
    if (!PyArg_ParseTuple(args, "si", &file, &verb))
        return NULL;
    scs = m6sc_per_file(file, 1, verb);
    m6sc_sr_accum();
    /* if (scs != 0) {
        PyErr_SetString(M6scError, "per file scan check failed");
        return NULL;
	} */
    return Py_BuildValue("i", scs);
}

/*
 * Get/set methods for scan check internals.
 */
static PyObject *m6sc_setopt(PyObject *self, PyObject *args)
{
    const char *kv;
    int rv;
    if (!PyArg_ParseTuple(args, "s", &kv))
        return NULL;
    rv = m6sc_set_chk_opt(kv);
    if (rv != 0) {
        PyErr_SetString(M6scError, "Unable to set option value");
        return NULL;
    }
    Py_RETURN_NONE;
}
static PyObject *m6sc_getopt(PyObject *self, PyObject *args)
{
    const char *arg;
    const char *val;
    if (!PyArg_ParseTuple(args, "s", &arg))
        return NULL;
    val = m6sc_get_chk_opt(arg);
    if (val == NULL) {
        PyErr_SetString(M6scError, "Unable to get option value");
        return NULL;
    }
    return Py_BuildValue("s", val);
}

/*
 * Bookends on scan check.
 */
static PyObject *m6sc_start(PyObject *self, PyObject *args)
{
    const char *scanref;
    int nf, mf;
    double tol;
    if (!PyArg_ParseTuple(args, "siid", &scanref, &nf, &mf, &tol))
        return NULL;
    m6sc_sr_start(scanref, nf, mf, tol);
    Py_RETURN_NONE;
}
static PyObject *m6sc_status(PyObject *self, PyObject *args)
{
    const char *scanref;
    const char *status;
    if (!PyArg_ParseTuple(args, "s", &scanref))
        return NULL;
    status = m6sc_sr_status(scanref);
    if (status == NULL) {
        PyErr_SetString(M6scError, "Unable retrieve scan status");
        return NULL;
    }
    return Py_BuildValue("s", status);
}

/*
 * For debugging, mostly.
 */
static PyObject *m6sc_scinfo(PyObject *self, PyObject *args)
{
    const char *details;
    details = m6sc_sr_scinfo();
    if (details == NULL) {
        PyErr_SetString(M6scError, "Unable retrieve scan check details");
    }
    return Py_BuildValue("s", details);
}

static PyMethodDef M6scMethods[] = {
    {"check_file",  m6sc_check_file, METH_VARARGS,
     "Run scan check on a single file."},
    {"setopt", m6sc_setopt, METH_VARARGS,
     "Set option for scan check."},
    {"getopt", m6sc_getopt, METH_VARARGS,
     "Get option for scan check."},
    {"start", m6sc_start, METH_VARARGS,
     "Declare a new scan:gref for check."},
    {"status", m6sc_status, METH_VARARGS,
     "Retrieve scan check status on group."},
    {"scinfo", m6sc_scinfo, METH_VARARGS,
     "Retrieve scan check internal details."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initm6sc(void)
{
    PyObject *m;

    m = Py_InitModule("m6sc", M6scMethods);
    /* we initialize the C checker here as well */
    if (m6sc_set_chk_opt(NULL) || m == NULL)
        return;

    M6scError = PyErr_NewException("m6sc.error", NULL, NULL);
    Py_INCREF(M6scError);
    PyModule_AddObject(m, "error", M6scError);
}

/*
 * eof
 */
