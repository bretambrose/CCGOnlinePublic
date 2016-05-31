/**********************************************************************************************************************

	(c) Copyright 2011, Bret Ambrose (mailto:bretambrose@gmail.com).

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************************************/

#include <IPCore/Debug/DebugAssert.h>

#include <IPCore/System/WindowsWrapper.h>

namespace IP
{
namespace Debug
{
namespace Assert {

	bool Modal_Assert_Dialog(const char *dialog_title, const char *dialog_text, AssertDialogType dialog_type) {
		DWORD flags = MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_DEFBUTTON3;
		switch (dialog_type) {
			case AssertDialogType.Ok:
				flags |= MB_OK;
				break;

			case AssertDialogType.YesNo:
				flags |= MB_YESNO;
				break;
		}

		return ::MessageBox(nullptr, dialog_text, dialog_title, flags) == IDYES;
	}

void Force_Debugger()
{
	DebugBreak();
}

}
}
}
