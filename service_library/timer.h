#pragma once

#include "framework.h"

// Когда таймер переходит в сигнальное положение, срабатывает этот колбек.
VOID CALLBACK timer_finish_callback( LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue );

// Асинхронный метод для запуска таймера.
void start_timer_async( LPVOID data );
