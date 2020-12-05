#include <Python/Python.h>
#include <iostream>
#include <string>

void PrintMyDef()
{
	PyObject *mydef = PyImport_ImportModule("python_scripts.python_script");
	if (mydef)
	{
		PyObject *printHello = PyObject_GetAttrString(mydef, "PrintMyDef");
		if (printHello)
		{
			PyObject *r = PyObject_CallFunction(printHello, NULL);
			if (r == Py_None)
			{
				std::cout << "None is returned." << std::endl;
				Py_XDECREF(r);
			}
			Py_XDECREF(printHello);
		}
		Py_XDECREF(mydef);
	}
}

int Multiply(int x, int y)
{
	PyObject *mydef = PyImport_ImportModule("python_scripts.python_script");
	int result;
	if (mydef)
	{
		PyObject *multiply = PyObject_GetAttrString(mydef, "Multiply");
		if (multiply)
		{
			PyObject *r = PyObject_CallFunction(multiply, (char *)"ii", x, y);
			if (r)
			{
				result = (int)PyInt_AS_LONG(r);
				Py_XDECREF(r);
			}
			Py_XDECREF(multiply);
		}
		Py_XDECREF(mydef);
	}
	return result;
}

void SysPath()
{
	std::cout << "sys.path: " << std::endl;
	PyRun_SimpleString("import sys\nprint sys.path\nprint\n");
}

int main(int argc, char **argv)
{
	Py_SetProgramName(argv[0]);
	Py_Initialize();
	std::cout << "GetProgramName: " << Py_GetProgramName() << std::endl;;
	if (Py_IsInitialized())
	{
		PySys_SetArgv(argc, argv);
		std::cout << "GetPath: " << Py_GetPath() << std::endl;
		SysPath();
		PrintMyDef(); // Calling python functions
		std::cout << "10 * 15 = " << Multiply(10, 15) << std::endl;
		Py_Finalize();
	}
	return 0;
}
