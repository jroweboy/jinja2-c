#include <Python.h>
#include <stdarg.h>

/**
 * Creates a new environment for Jinja and returns a pointer to the PyObject
 * Before the program exits you should call free_environment to all python
 * and c to clean up.
 * This function takes a string for the absolute directory to the template directory
 */

 // TODO refactor this to make clean up easier
PyObject* init_environment(const char* template_dir) {
    PyObject *pString, *pmodEnvironment, *pmodFileSystemLoader, *pDict;
    PyObject *pClassEnv, *pClassFLoader, *pArgs, *pFileLoader, *ret_env;
    PyObject *empty_tuple;

    // from jinja2 import Environment, FileSystemLoader
Py_Initialize();
    pString = PyString_FromString("jinja2.environment");
    pmodEnvironment = PyImport_Import(pString);
    Py_DECREF(pString);
    if (!pmodEnvironment) {
        fprintf(stderr, "Error finding module jinja2\n");
        PyErr_Print();
        Py_Finalize();
        return NULL;
    }
    pString = PyString_FromString("jinja2.loaders");
    pmodFileSystemLoader = PyImport_Import(pString);
    Py_DECREF(pString);

    if (!pmodEnvironment) {
        fprintf(stderr, "Error finding module jinja2\n");
        PyErr_Print();
        Py_Finalize();
        return NULL;
    }
    // env = Environment(loader=FileSystemLoader(template_dir))
    // Get a reference to the Environment class
    pDict = PyModule_GetDict(pmodEnvironment);
    pClassEnv = PyDict_GetItemString(pDict, "Environment");

    if (!pClassEnv || !PyCallable_Check(pClassEnv)){
        fprintf(stderr, "Could not import Environment\n");
        PyErr_Print();
        Py_DECREF(pmodEnvironment);
        Py_DECREF(pmodFileSystemLoader);
        Py_Finalize();
        return NULL;
    }

    // get a reference to the FileSystemLoader class
    pDict = PyModule_GetDict(pmodFileSystemLoader);
    pClassFLoader = PyDict_GetItemString(pDict, "FileSystemLoader");

    if (!pClassFLoader || ! PyCallable_Check(pClassFLoader)) {
        fprintf(stderr, "Could not import FileSystemLoader\n");
        PyErr_Print();
        Py_DECREF(pClassEnv);
        Py_DECREF(pmodEnvironment);
        Py_DECREF(pmodFileSystemLoader);
        Py_Finalize();
        return NULL;
    }

    // now make an instance of the FileSystemLoader to pass to the Environment
    pArgs = Py_BuildValue("(s)", template_dir);
    pFileLoader = PyObject_Call(pClassFLoader, pArgs, NULL);
    Py_DECREF(pArgs);
    if (pFileLoader == NULL) {
        fprintf(stderr, "Could not instantiate a new FileSystemLoader\n");
        PyErr_Print();
        Py_DECREF(pClassEnv);
        Py_DECREF(pClassFLoader);
        Py_DECREF(pmodEnvironment);
        Py_DECREF(pmodFileSystemLoader);
        return NULL;
    }

    // Create an instance of the Environment class
    empty_tuple = PyTuple_New(0);
    pArgs = Py_BuildValue("{sO}", "loader", pFileLoader);
    ret_env = PyObject_Call(pClassEnv, empty_tuple, pArgs);
    Py_DECREF(pArgs);
    Py_DECREF(empty_tuple);

    if (ret_env == NULL) {
        fprintf(stderr, "Could not instantiate a new Environment\n");
        PyErr_Print();
        Py_DECREF(pFileLoader);
        Py_DECREF(pClassEnv);
        Py_DECREF(pClassFLoader);
        Py_DECREF(pmodEnvironment);
        Py_DECREF(pmodFileSystemLoader);
        return NULL;
    }
    Py_DECREF(pFileLoader);
    Py_DECREF(pClassEnv);
    Py_DECREF(pClassFLoader);
    Py_DECREF(pmodEnvironment);
    Py_DECREF(pmodFileSystemLoader);
    return ret_env;
}

void free_environment(PyObject* env) {
    if (env) {
        Py_DECREF(env);
        Py_Finalize();
    }
}

PyObject* get_template(PyObject* env, const char* template_name) {
    PyObject *retval;
    retval = PyObject_CallMethod(env, "get_template", "(s)", template_name);
    if (!retval) {
        fprintf(stderr, "Could not load template %s\n", template_name);
        // PyErr_Print();
        return NULL;
    }
    return retval;
}

// TODO support optional extensions and filter_func
void list_templates(PyObject* env) {
    PyObject *retval, *iterator, *item;
    retval = PyObject_CallMethod(env, "list_templates", NULL);
    if (!retval) {
        fprintf(stderr, "Could not list templates \n");
        PyErr_Print();
        return;
    }

    iterator = PyObject_GetIter(retval);
    if (iterator == NULL) {
        fprintf(stderr, "Could not list templates \n");
        PyErr_Print();
        Py_DECREF(retval);
        return;
    }

    while ((item = PyIter_Next(iterator))) {
        printf("%s\n", PyString_AsString(item));
        Py_DECREF(item);
    }

    Py_DECREF(iterator);

    if (PyErr_Occurred()) {
        PyErr_Print();
        Py_DECREF(retval);
    }
}

/**
 * Calls the render function on this template and returns the rendered string. 
 * Do not call free on the rendered string!
 *
 * int count, the number of string arguments provided to that function
 * var_args must be of type char *
 * ex: 
 * render(tmpl, 2, "name", "James");
 * render(tmpl, 4, "name", "James", "age", "24");
 */
char* render(PyObject *tmpl, int count, ...) {
    PyObject *retval, *pArgs, *pString;
    va_list ap;
    char *format_str;
    int i;

    if (count % 2) {
        fprintf(stderr, "The number of items given must be  \n");
        return NULL;
    }

    // make a format string which looks like {ss,ss,..ss}\0
    // +3       for the { }\0
    // +count/2 for the commas
    // -1       for omitting the trailing comma
    format_str = (char*) malloc( sizeof(char) * count + (count/2) + 2);
    for (i=1; i<=count; ++i) {
        format_str[i] = 's';
        if (i%2==0 && i != count) {
            format_str[++i] = ',';
        }
    }
    format_str[0] = '{';
    format_str[count+1] = '}';
    format_str[count+2] = '\0';

    va_start(ap, count);
    pArgs = Py_VaBuildValue(format_str, ap);
    va_end(ap);

    free(format_str);

    pString = PyString_FromString("render");
    retval = PyObject_CallMethodObjArgs(tmpl, pString, pArgs, NULL);
    Py_DECREF(pString);
    Py_DECREF(pArgs);
    if (!retval) {
        fprintf(stderr, "Could not list templates \n");
        PyErr_Print();
        return NULL;
    }
    return PyString_AsString(retval);
}

int main(int argc, char *argv[])
{
    PyObject *env, *tmpl;
    char *rendered;

    env = init_environment("/home/jrowe7/slf/jinja2-c/test_template/");
    if (!env) {
        fprintf(stderr, "Could not instantiate a new Environment\n");
        return -1;
    }

    list_templates(env);

    tmpl = get_template(env, "index.html");
    // badval = get_template(env, "dne");
    rendered = render(tmpl, 2, "name", "James");
    if (rendered) {
        printf("%s\n", rendered);
    }
    free_environment(env);
    return 0;
}