//
// VMime library (http://vmime.sourceforge.net)
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

#include "vmime/encoderB64.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime
{


encoderB64::encoderB64()
{
}


const std::vector <string> encoderB64::getAvailableProperties() const
{
	std::vector <string> list(encoder::getAvailableProperties());

	list.push_back("maxlinelength");

	return (list);
}


// 7-bits alphabet used to encode binary data
const unsigned char encoderB64::sm_alphabet[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

const unsigned char encoderB64::sm_decodeMap[256] =
{
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0x00 - 0x0f
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0x10 - 0x1f
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,  // 0x20 - 0x2f
	0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0x3d,0xff,0xff,  // 0x30 - 0x3f
	0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,  // 0x40 - 0x4f
	0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,  // 0x50 - 0x5f
	0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,  // 0x60 - 0x6f
	0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff,  // 0x70 - 0x7f
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0x80 - 0x8f
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0x90 - 0x9f
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0xa0 - 0xaf
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0xb0 - 0xbf
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0xc0 - 0xcf
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0xd0 - 0xdf
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0xe0 - 0xef
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  // 0xf0 - 0xff
};



const utility::stream::size_type encoderB64::encode(utility::inputStream& in, utility::outputStream& out)
{
	in.reset();  // may not work...

	const int propMaxLineLength = getProperties().getProperty <int>("maxlinelength", -1);

	const bool cutLines = (propMaxLineLength != -1);
	const int maxLineLength = std::min(propMaxLineLength, 76);

	// Process data
	utility::stream::value_type buffer[65536];
	utility::stream::size_type bufferLength = 0;
	utility::stream::size_type bufferPos = 0;

	unsigned char bytes[3];
	unsigned char output[4];

	utility::stream::size_type total = 0;

	int curCol = 0;

	while (bufferPos < bufferLength || !in.eof())
	{
		if (bufferPos >= bufferLength)
		{
			bufferLength = in.read(buffer, sizeof(buffer));
			bufferPos = 0;

			if (bufferLength == 0)
				break;
		}

		// Get 3 bytes of data
		int count = 0;

		while (count < 3 && bufferPos < bufferLength)
			bytes[count++] = buffer[bufferPos++];

		if (count != 3)
		{
			// There may be more data in the next chunk...
			if (bufferPos >= bufferLength)
			{
				bufferLength = in.read(buffer, sizeof(buffer));
				bufferPos = 0;
			}

			while (count < 3 && bufferPos < bufferLength)
				bytes[count++] = buffer[bufferPos++];
		}

		// Encode data
		switch (count)
		{
		case 1:

			output[0] = sm_alphabet[(bytes[0] & 0xFC) >> 2];
			output[1] = sm_alphabet[(bytes[0] & 0x03) << 4];
			output[2] = sm_alphabet[64]; // padding
			output[3] = sm_alphabet[64]; // padding

			break;

		case 2:

			output[0] = sm_alphabet[(bytes[0] & 0xFC) >> 2];
			output[1] = sm_alphabet[((bytes[0] & 0x03) << 4) | ((bytes[1] & 0xF0) >> 4)];
			output[2] = sm_alphabet[(bytes[1] & 0x0F) << 2];
			output[3] = sm_alphabet[64]; // padding

			break;

		default:
		case 3:

			output[0] = sm_alphabet[(bytes[0] & 0xFC) >> 2];
			output[1] = sm_alphabet[((bytes[0] & 0x03) << 4) | ((bytes[1] & 0xF0) >> 4)];
			output[2] = sm_alphabet[((bytes[1] & 0x0F) << 2) | ((bytes[2] & 0xC0) >> 6)];
			output[3] = sm_alphabet[(bytes[2] & 0x3F)];

			break;
		}

		// Write encoded data to output stream
		out.write((char*) output, 4);

		total += 4;
		curCol += 4;

		if (cutLines && curCol >= maxLineLength - 1)
		{
			out.write("\r\n", 2);
			curCol = 0;
		}
	}

	return (total);
}


const utility::stream::size_type encoderB64::decode(utility::inputStream& in, utility::outputStream& out)
{
	in.reset();  // may not work...

	// Process the data
	char buffer[16384];
	int bufferLength = 0;
	int bufferPos = 0;

	utility::stream::size_type total = 0;

	unsigned char bytes[4];
	unsigned char output[3];

	while (bufferPos < bufferLength || !in.eof())
	{
		// Need to get more data?
		if (bufferPos >= bufferLength)
		{
			bufferLength = in.read(buffer, sizeof(buffer));
			bufferPos = 0;

			// No more data
			if (bufferLength == 0)
				break;
		}

		// 4 bytes of input provide 3 bytes of output, so
		// get the next 4 bytes from the input stream.
		int count = 0;

		while (count < 4 && bufferPos < bufferLength)
		{
			const unsigned char c = buffer[bufferPos++];

			if (!isspace(c))
				bytes[count++] = c;
		}

		if (count != 4)
		{
			while (count < 4 && !in.eof())
			{
				// Data continues on the next chunk
				bufferLength = in.read(buffer, sizeof(buffer));
				bufferPos = 0;

				while (count < 4 && bufferPos < bufferLength)
				{
					const unsigned char c = buffer[bufferPos++];

					if (!isspace(c))
						bytes[count++] = c;
				}
			}
		}

		// Decode the bytes
		unsigned char c1 = bytes[0];
		unsigned char c2 = bytes[1];

		if (c1 == '=' || c2 == '=')  // end
			break;

		output[0] = (unsigned char)((sm_decodeMap[c1] << 2) | ((sm_decodeMap[c2] & 0x30) >> 4));

		c1 = bytes[2];

		if (c1 == '=')  // end
		{
			out.write((char*) output, 1);
			total += 1;
			break;
		}

		output[1] = (unsigned char)(((sm_decodeMap[c2] & 0xf) << 4) | ((sm_decodeMap[c1] & 0x3c) >> 2));

		c2 = bytes[3];

		if (c2 == '=')  // end
		{
			out.write((char*) output, 2);
			total += 2;
			break;
		}

		output[2] = (unsigned char)(((sm_decodeMap[c1] & 0x03) << 6) | sm_decodeMap[c2]);

		out.write((char*) output, 3);
		total += 3;
	}

	return (total);
}


} // vmime
