/**
 * WinPR: Windows Portable Runtime
 * WinPR Logger
 *
 * Copyright 2013 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <winpr/crt.h>
#include <winpr/path.h>

#include <winpr/wlog.h>

#include "wlog/Message.h"

#include "wlog/ConsoleAppender.h"

/**
 * Console Appender
 */

void WLog_ConsoleAppender_SetOutputStream(wLog* log, wLogConsoleAppender* appender, int outputStream)
{
	if (!appender)
		return;

	if (appender->Type != WLOG_APPENDER_CONSOLE)
		return;

	if (outputStream < 0)
		outputStream = WLOG_CONSOLE_STDOUT;

	if (outputStream == WLOG_CONSOLE_STDOUT)
		appender->outputStream = WLOG_CONSOLE_STDOUT;
	else if (outputStream == WLOG_CONSOLE_STDERR)
		appender->outputStream = WLOG_CONSOLE_STDERR;
	else
		appender->outputStream = WLOG_CONSOLE_STDOUT;
}

int WLog_ConsoleAppender_Open(wLog* log, wLogConsoleAppender* appender)
{
	return 0;
}

int WLog_ConsoleAppender_Close(wLog* log, wLogConsoleAppender* appender)
{
	return 0;
}

int WLog_ConsoleAppender_WriteMessage(wLog* log, wLogConsoleAppender* appender, wLogMessage* message)
{
	FILE* fp;
	char prefix[WLOG_MAX_PREFIX_SIZE];

	fp = (appender->outputStream == WLOG_CONSOLE_STDERR) ? stderr : stdout;

	message->PrefixString = prefix;
	WLog_Layout_GetMessagePrefix(log, appender->Layout, message);

	fprintf(fp, "%s%s\n", message->PrefixString, message->TextString);

	return 1;
}

static int g_DataId = 0;

int WLog_ConsoleAppender_WriteDataMessage(wLog* log, wLogConsoleAppender* appender, wLogMessage* message)
{
	int DataId;
	char* FullFileName;

	DataId = g_DataId++;
	FullFileName = WLog_Message_GetOutputFileName(DataId, "dat");

	WLog_DataMessage_Write(FullFileName, message->Data, message->Length);

	free(FullFileName);

	return DataId;
}

static int g_ImageId = 0;

int WLog_ConsoleAppender_WriteImageMessage(wLog* log, wLogConsoleAppender* appender, wLogMessage* message)
{
	int ImageId;
	char* FullFileName;

	ImageId = g_ImageId++;
	FullFileName = WLog_Message_GetOutputFileName(ImageId, "bmp");

	WLog_ImageMessage_Write(FullFileName, message->ImageData,
			message->ImageWidth, message->ImageHeight, message->ImageBpp);

	free(FullFileName);

	return ImageId;
}

static int g_PacketId = 0;

int WLog_ConsoleAppender_WritePacketMessage(wLog* log, wLogConsoleAppender* appender, wLogMessage* message)
{
	int PacketId;
	char* FullFileName;

	PacketId = g_PacketId++;

	if (!appender->PacketMessageContext)
	{
		FullFileName = WLog_Message_GetOutputFileName(-1, "pcap");
		appender->PacketMessageContext = (void*) Pcap_Open(FullFileName, TRUE);
		free(FullFileName);
	}

	WLog_PacketMessage_Write((wPcap*) appender->PacketMessageContext,
			message->PacketData, message->PacketLength, message->PacketFlags);

	return PacketId;
}

wLogConsoleAppender* WLog_ConsoleAppender_New(wLog* log)
{
	wLogConsoleAppender* ConsoleAppender;

	ConsoleAppender = (wLogConsoleAppender*) malloc(sizeof(wLogConsoleAppender));

	if (ConsoleAppender)
	{
		ZeroMemory(ConsoleAppender, sizeof(wLogConsoleAppender));

		ConsoleAppender->Type = WLOG_APPENDER_CONSOLE;

		ConsoleAppender->Open = (WLOG_APPENDER_OPEN_FN) WLog_ConsoleAppender_Open;
		ConsoleAppender->Close = (WLOG_APPENDER_OPEN_FN) WLog_ConsoleAppender_Close;

		ConsoleAppender->WriteMessage =
				(WLOG_APPENDER_WRITE_MESSAGE_FN) WLog_ConsoleAppender_WriteMessage;
		ConsoleAppender->WriteDataMessage =
				(WLOG_APPENDER_WRITE_DATA_MESSAGE_FN) WLog_ConsoleAppender_WriteDataMessage;
		ConsoleAppender->WriteImageMessage =
				(WLOG_APPENDER_WRITE_IMAGE_MESSAGE_FN) WLog_ConsoleAppender_WriteImageMessage;
		ConsoleAppender->WritePacketMessage =
				(WLOG_APPENDER_WRITE_PACKET_MESSAGE_FN) WLog_ConsoleAppender_WritePacketMessage;

		ConsoleAppender->outputStream = WLOG_CONSOLE_STDOUT;
	}

	return ConsoleAppender;
}

void WLog_ConsoleAppender_Free(wLog* log, wLogConsoleAppender* appender)
{
	if (appender)
	{
		if (appender->PacketMessageContext)
		{
			Pcap_Close((wPcap*) appender->PacketMessageContext);
		}

		free(appender);
	}
}
