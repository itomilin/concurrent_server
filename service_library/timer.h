#pragma once

#include "framework.h"

// ����� ������ ��������� � ���������� ���������, ����������� ���� ������.
VOID CALLBACK timer_finish_callback( LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue );

// ����������� ����� ��� ������� �������.
void start_timer_async( LPVOID data );
