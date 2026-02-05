/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
#include <gs/thirdparty/htpa/htpa32x32d.h>
#include <Python.h>
#include <time.h>
#if PY_MAJOR_VERSION == 3
#define IS_PY3
#endif

// static versions of data to avoid passing the back and forth in the simple usecase
static gs_htpa_th_data_t th_data;

static PyObject* htpa_uint_data_to_python_list(uint16_t *data, uint32_t length)
{
    PyObject* lst = PyList_New(length);
    for (uint32_t i = 0; i < length; i++)
    {
        PyList_SET_ITEM(lst, i, Py_BuildValue("I", data[i]));
    }
    return lst;
}

static PyObject* htpa_int_data_to_python_list(int16_t *data, uint32_t length)
{
    PyObject* lst = PyList_New(length);
    for (uint32_t i = 0; i < length; i++)
    {
        PyList_SET_ITEM(lst, i, Py_BuildValue("i", data[i]));
    }
    return lst;
}

static PyObject* pyhtpa_configure(PyObject* self, PyObject* args)
{
    uint8_t bus;
    uint8_t address;
    uint8_t eeprom_address;
    if (!PyArg_ParseTuple(args, "bbb", &bus, &address, &eeprom_address))
    {
        Py_RETURN_NONE;
    }

    gs_htpa_handle_t* handle = malloc(sizeof(gs_htpa_handle_t));

    printf("TEST %d %d %d\n", bus, address, eeprom_address);
    gs_error_t error = gs_htpa_configure(handle, bus, address, eeprom_address);
    printf("HEST %d %d %d\n", handle->htpa_bus, handle->htpa_address, handle->htpa_eeprom_address);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("iO", error, PyCapsule_New(handle, "gs_htpa_handle_t", NULL));
}

static PyObject* pyhtpa_mbit_ref_cal(PyObject* self, PyObject* args)
{
    uint8_t mbit_trim;
    uint8_t ref_cal;
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "Obb", &handle, &mbit_trim, &ref_cal))
    {
        Py_RETURN_NONE;
    }
    gs_error_t error = gs_htpa_mbit_ref_cal(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            mbit_trim, ref_cal);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("i", error);
}

static PyObject* pyhtpa_bias_trim(PyObject* self, PyObject* args)
{
    uint8_t bias_top;
    uint8_t bias_bottom;
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "Obb", &handle, &bias_top, &bias_bottom))
    {
        Py_RETURN_NONE;
    }
    gs_error_t error = gs_htpa_bias_trim(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            bias_top, bias_bottom);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("i", error);
}

static PyObject* pyhtpa_clock_trim(PyObject* self, PyObject* args)
{
    uint8_t clockfreq;
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "Ob", &handle, &clockfreq))
    {
        Py_RETURN_NONE;
    }
    gs_error_t error = gs_htpa_clock_trim(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            clockfreq);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("i", error);
}

static PyObject* pyhtpa_bpa_trim(PyObject* self, PyObject* args)
{
    uint8_t bpa_top;
    uint8_t bpa_bottom;
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "Obb", &handle, &bpa_top, &bpa_bottom))
    {
        Py_RETURN_NONE;
    }
    gs_error_t error = gs_htpa_bpa_trim(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            bpa_top, bpa_bottom);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("i", error);
}

static PyObject* pyhtpa_wakeup(PyObject* self, PyObject *args)
{
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "O", &handle))
    {
        Py_RETURN_NONE;
    }
    gs_error_t error = gs_htpa_wakeup(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")));
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("i", error);
}
static PyObject* pyhtpa_get_image(PyObject* self, PyObject *args)
{
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "O", &handle))
    {
        Py_RETURN_NONE;
    }
    gs_htpa_image_t image;
    uint16_t mean_ptat;
    gs_error_t error = gs_htpa_read_image(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            &image, &mean_ptat);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("iOI", error, htpa_uint_data_to_python_list(image.image, 1024), mean_ptat);
}

static PyObject* pyhtpa_get_blind(PyObject* self, PyObject* args)
{
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "O", &handle))
    {
        Py_RETURN_NONE;
    }
    gs_htpa_blind_image_t blind;
    gs_error_t error = gs_htpa_read_blind_image(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            &blind);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("iO", error, htpa_uint_data_to_python_list(blind.image, 256));
}

static PyObject* pyhtpa_read_factory_calibration(PyObject* self, PyObject* args)
{
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "O", &handle))
    {
        Py_RETURN_NONE;
    }
    uint8_t mbit, bias, clock, bpa, pu, ref_cal;
    gs_error_t error = gs_htpa_read_factory_calibration(
            *((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            &mbit, &ref_cal, &bias, &clock, &bpa, &pu);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("ibbbbbb", error, mbit, ref_cal, bias, clock, bpa, pu);
}

static PyObject* pyhtpa_read_th_grad(PyObject* self, PyObject* args)
{
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "O", &handle))
    {
        Py_RETURN_NONE;
    }
    gs_htpa_th_data_t data;
    gs_error_t error = gs_htpa_read_th_grad(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            &data);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("iO", error, data);
}


static PyObject* pyhtpa_read_th_offset(PyObject* self, PyObject* args)
{
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "O", &handle))
    {
        Py_RETURN_NONE;
    }
    gs_htpa_th_data_t data;
    gs_error_t error = gs_htpa_read_th_offset(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            &data);
    if (error)
    {
        printf("Error in call %s\n", __FUNCTION__);
    }
    return Py_BuildValue("iO", error, data);
}

static PyObject* pyhtpa_start_and_default_configure(PyObject* self, PyObject *args)
{
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "O", &handle))
    {
        Py_RETURN_NONE;
    }
    gs_error_t error;
    //wake up
    error = gs_htpa_wakeup(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")));
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }

    //read eeprom
    uint8_t mbit, bias, clock, bpa, pu, refcal;
    error = gs_htpa_read_factory_calibration(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            &mbit, &refcal, &bias, &clock, &bpa, &pu);
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }
    
    //set config
    error = gs_htpa_mbit_ref_cal(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")), mbit, refcal);
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }
    error = gs_htpa_clock_trim(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")), clock);
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }
    error = gs_htpa_bpa_trim(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")), bpa, bpa);
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }
    // Not setting bias and pu

    //read th data and gradscalepow

    error = gs_htpa_read_th_offset(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")), &th_data);
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }
    error = gs_htpa_read_th_grad(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")), &th_data);
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }
    error = gs_htpa_read_grad_scale_pow(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")),
            &th_data);
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }
    return Py_BuildValue("i", error);
}

static PyObject* pyhtpa_read_image_with_blind(PyObject* self, PyObject *args)
{
    PyObject* handle;
    if (!PyArg_ParseTuple(args, "O", &handle))
    {
        Py_RETURN_NONE;
    }
    gs_htpa_image_t image;
    gs_htpa_blind_image_t blind;
    gs_error_t error;
    uint16_t m_ptat;

    error = gs_htpa_read_image(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")), &image, &m_ptat);
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }
    error = gs_htpa_read_blind_image(*((gs_htpa_handle_t*)PyCapsule_GetPointer(handle, "gs_htpa_handle_t")), &blind);
    if (error)
    {
        printf("Error in call %s line %d\n", __FUNCTION__, __LINE__);
    }
    gs_htpa_adjusted_image_t result;
    gs_htpa_adjust_image(&image, &blind, &th_data, m_ptat, &result);
    return Py_BuildValue("iO", error, htpa_int_data_to_python_list(result.image, 1024));

}

static PyMethodDef methods[] = {
    {"wakeup", pyhtpa_wakeup, METH_VARARGS, ""},
    {"get_image", pyhtpa_get_image, METH_VARARGS, ""},
    {"get_blind", pyhtpa_get_blind, METH_VARARGS, ""},
    {"bpa_trim", pyhtpa_bpa_trim, METH_VARARGS, ""},
    {"clock_trim", pyhtpa_clock_trim, METH_VARARGS, ""},
    {"bias_trim", pyhtpa_bias_trim, METH_VARARGS, ""},
    {"mbit_ref_cal", pyhtpa_mbit_ref_cal, METH_VARARGS, ""},
    {"configure", pyhtpa_configure, METH_VARARGS, ""},
    {"read_factory_calibration", pyhtpa_read_factory_calibration, METH_VARARGS, ""},
    {"read_th_grad", pyhtpa_read_th_grad, METH_VARARGS, ""},
    {"read_th_offset", pyhtpa_read_th_offset, METH_VARARGS, ""},
    {"start_and_default_configure", pyhtpa_start_and_default_configure, METH_VARARGS, ""},
    {"read_image_with_blind", pyhtpa_read_image_with_blind, METH_VARARGS, ""},
    /* sentinel */
    {NULL, NULL, 0, NULL}
};

#ifdef IS_PY3
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "pygsaardvark_py3",
    NULL, /* module documentation, may be NULL */
    -1,   /* size of per-interpreter state of the module,
             or -1 if the module keeps state in global variables. */
    methods,
    NULL,
    NULL,
    NULL,
    NULL
};
#endif

#ifdef IS_PY3
PyMODINIT_FUNC PyInit_libgsaardvark_py3(void) {
#else
PyMODINIT_FUNC initlibgsthirdparty_py2(void) {
#endif

#ifdef IS_PY3
    PyObject* m = PyModule_Create(&moduledef);
#else
    Py_InitModule("libgsthirdparty_py2", methods);
#endif

#ifdef IS_PY3
    return m;
#endif
}
