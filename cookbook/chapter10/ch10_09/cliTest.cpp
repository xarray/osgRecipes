#include "stdafx.h"
#include "osgWindow.h"

using namespace System;
using namespace System::Windows::Forms;

int main(array<System::String ^> ^args)
{
    cliTest::osgWindow^ form = gcnew cliTest::osgWindow();
	Application::Run(form);
    return 0;
}
