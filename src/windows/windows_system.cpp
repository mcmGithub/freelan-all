/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file windows_system.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief Windows system primitives.
 */

#include "windows/windows_system.hpp"

#include <shlobj.h>
#include <shellapi.h>

namespace asiotap
{
	namespace
	{
		class handle_closer
		{
			public:
				handle_closer(HANDLE handle) : m_handle(handle) {}
				~handle_closer() { ::CloseHandle(m_handle); }

			private:
				HANDLE m_handle;
		};

		DWORD do_create_process(const char* application, char* command_line, STARTUPINFO& si, PROCESS_INFORMATION& pi)
		{
			return ::CreateProcessA(application, command_line, NULL, NUL, FALSE, 0, NULL, NULL, &si, &pi);
		}

		DWORD do_create_process(const wchar_t* application, wchar_t* command_line, STARTUPINFO& si, PROCESS_INFORMATION& pi)
		{
			return ::CreateProcessW(application, command_line, NULL, NUL, FALSE, 0, NULL, NULL, &si, &pi);
		}

		template <typename CharType>
		DWORD create_process(const CharType* application, CharType* command_line)
		{
			DWORD exit_status;

			STARTUPINFO si;
			si.cb = sizeof(si);
			si.lpReserved = NULL;
			si.lpDesktop = NULL;
			si.lpTitle = NULL;
			si.dwX = 0;
			si.dwY = 0;
			si.dwXSize = 0;
			si.dwYSize = 0;
			si.dwXCountChars = 0;
			si.dwYCountChars = 0;
			si.dwFillAttribute = 0;
			si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; // Remove STARTF_USESTDHANDLES to show stdout
			si.wShowWindow = SW_HIDE;
			si.cbReserved2 = 0;
			si.lpReserved2 = NULL;
			si.hStdInput = INVALID_HANDLE_VALUE;
#if FREELAN_DEBUG
			si.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
#else
			si.hStdOutput = INVALID_HANDLE_VALUE;
#endif
			si.hStdError = INVALID_HANDLE_VALUE;

			PROCESS_INFORMATION pi;

			if (!do_create_process(application, command_line, si, pi))
			{
				throw boost::system::system_error(::GetLastError(), boost::system::system_category());
			}

			handle_closer thread_closer(pi.hThread);
			handle_closer process_closer(pi.hProcess);

			DWORD wait_result = ::WaitForSingleObject(pi.hProcess, INFINITE);

			switch (wait_result)
			{
				case WAIT_OBJECT_0:
					{
						DWORD exit_code = 0;

						if (::GetExitCodeProcess(pi.hProcess, &exit_code))
						{
							exit_status = static_cast<int>(exit_code);
						}
						else
						{
							throw boost::system::system_error(::GetLastError(), boost::system::system_category());
						}

						break;
					}
				default:
					{
						throw boost::system::system_error(::GetLastError(), boost::system::system_category());
					}
			}

			return exit_status;
		}
	}

	int execute(const std::vector<std::string>& args, boost::system::error_code& ec)
	{
		//TODO: Implement
		return EXIT_FAILURE;
	}

	int execute(const std::vector<std::string>& args)
	{
		boost::system::error_code ec;

		const auto result = execute(args, ec);

		if (result < 0)
		{
			throw boost::system::system_error(ec);
		}

		return result;
	}

	void checked_execute(const std::vector<std::string>& args)
	{
		if (execute(args) != 0)
		{
			throw boost::system::system_error(make_error_code(asiotap_error::external_process_failed));
		}
	}
}
