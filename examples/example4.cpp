//
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

//
// EXAMPLE DESCRIPTION:
// ====================
// This sample program demonstrate the use of the messageParser component
// to enumerate the text parts in a message.
//
// For more information, please visit:
// http://vmime.sourceforge.net/
//

#include <iostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/handler.hpp"


int main()
{
	std::cout << std::endl;

	// VMime initialization
	vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

	try
	{
		vmime::messageParser mp("<...MIME message content...>");

		// Enumerate text parts
		for (int i = 0 ; i < mp.getTextPartCount() ; ++i)
		{
			const vmime::textPart& part = *mp.getTextPartAt(i);

			// Output content-type of the part
			std::cout << part.getType().generate() << std::endl;

			// text/html
			if (part.getType().getSubType() == vmime::mediaTypes::TEXT_HTML)
			{
				const vmime::htmlTextPart& hp = dynamic_cast<const vmime::htmlTextPart&>(part);

				// HTML text is in "hp.getText()"
				// Corresponding plain text is in "hp.getPlainText()"

				// Enumerate embedded objects (eg. images)
				for (int j = 0 ; j < hp.getObjectCount() ; ++j)
				{
					const vmime::htmlTextPart::embeddedObject& obj = *hp.getObjectAt(j);

					// Identifier (content-id or content-location) is in "obj.getId()"
					// Object data is in "obj.getData()"
				}
			}
			// text/plain
			else
			{
				const vmime::textPart& tp = dynamic_cast<const vmime::textPart&>(part);

				// Text is in "tp.getText()"
			}
		}
	}
	// VMime exception
	catch (vmime::exception& e)
	{
		std::cout << "vmime::exception: " << e.what() << std::endl;
		throw;
	}
	// Standard exception
	catch (std::exception& e)
	{
		std::cout << "std::exception: " << e.what() << std::endl;
		throw;
	}

	std::cout << std::endl;
}
