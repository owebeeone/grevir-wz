@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
if not exist build\irq-msvc mkdir build\irq-msvc
set CXX_FLAGS=/nologo /std:c++latest /DHAS_STD_LIB=1 /I. /Igrevir-base\src /Igrevir-core\src /Igrevir-peripherals\src /Igrevir-test-support\include /Iscratch\interrupt-implementation-gates
cl %CXX_FLAGS% /Zs /DGREVIR_IRQ_PROBE=1 scratch\interrupt-implementation-gates\binding_probe.cpp
if errorlevel 1 exit /b 2
cl %CXX_FLAGS% /Zs scratch\interrupt-implementation-gates\timer_allocator_probe.cpp
if errorlevel 1 exit /b 8
cl %CXX_FLAGS% /Zs /DGREVIR_IRQ_PROBE=1 scratch\interrupt-implementation-gates\inventory_probe.cpp
if errorlevel 1 exit /b 12
cl %CXX_FLAGS% /Zs scratch\interrupt-implementation-gates\undemanded_source_probe.cpp
if errorlevel 1 exit /b 13
cl %CXX_FLAGS% /Zs scratch\interrupt-implementation-gates\duplicate_catalog.cpp >build\irq-msvc\duplicate_catalog.log 2>&1
if not errorlevel 1 exit /b 9
cl %CXX_FLAGS% /c /DGREVIR_IRQ_PROBE=1 /Fobuild\irq-msvc\mock_record.obj scratch\interrupt-implementation-gates\mock_record.cpp
if errorlevel 1 exit /b 3
"C:\Users\gianni\AppData\Local\Programs\Python\Python313\python.exe" -B grevir-core\tools\grevir_irqgen\__main__.py plan --object build\irq-msvc\mock_record.obj --out-dir build\irq-msvc --attempt native-msvc --backend mock --target mock_mcu --board mock_board --compiler scratch_compiler
if errorlevel 1 exit /b 4
"C:\Users\gianni\AppData\Local\Programs\Python\Python313\python.exe" -B grevir-core\tools\grevir_irqgen\__main__.py emit --out-dir build\irq-msvc --attempt native-msvc --backend mock --compiler scratch_compiler --application-header scratch/interrupt-implementation-gates/mock_app.hpp
if errorlevel 1 exit /b 5
cl %CXX_FLAGS% /Ibuild\irq-msvc /DGREVIR_GENERATED_IRQ_HEADER=\"grevir_generated_irq_bindings_mock.hpp\" scratch\interrupt-implementation-gates\mock_main.cpp build\irq-msvc\grevir_generated_irq_bindings_mock.cpp /Febuild\irq-msvc\mock_firmware.exe
if errorlevel 1 exit /b 6
cl %CXX_FLAGS% /Zs /Ibuild\irq-msvc /DGREVIR_GENERATED_IRQ_HEADER=\"grevir_generated_irq_bindings_mock.hpp\" scratch\interrupt-implementation-gates\unbound_handler.cpp >build\irq-msvc\unbound.log 2>&1
if not errorlevel 1 exit /b 10
cl %CXX_FLAGS% /Zs /Ibuild\irq-msvc /DGREVIR_GENERATED_IRQ_HEADER=\"grevir_generated_irq_bindings_mock.hpp\" scratch\interrupt-implementation-gates\foreign_same_key.cpp >build\irq-msvc\foreign.log 2>&1
if not errorlevel 1 exit /b 11
build\irq-msvc\mock_firmware.exe
if errorlevel 1 exit /b 7
echo PASS native MSVC probe, JSON, generated unit and dispatch
if not exist build\irq-msvc\event mkdir build\irq-msvc\event
cl %CXX_FLAGS% /c /DGREVIR_IRQ_PROBE=1 /Fobuild\irq-msvc\event\mock_event_record.obj scratch\interrupt-implementation-gates\mock_event_record.cpp
if errorlevel 1 exit /b 14
"C:\Users\gianni\AppData\Local\Programs\Python\Python313\python.exe" -B grevir-core\tools\grevir_irqgen\__main__.py plan --object build\irq-msvc\event\mock_event_record.obj --out-dir build\irq-msvc\event --attempt native-msvc-event --backend mock --target mock_mcu --board mock_board --compiler scratch_compiler
if errorlevel 1 exit /b 15
"C:\Users\gianni\AppData\Local\Programs\Python\Python313\python.exe" -B grevir-core\tools\grevir_irqgen\__main__.py emit --out-dir build\irq-msvc\event --attempt native-msvc-event --backend mock --compiler scratch_compiler --application-header scratch/interrupt-implementation-gates/mock_event_app.hpp
if errorlevel 1 exit /b 16
cl %CXX_FLAGS% /Ibuild\irq-msvc\event /DGREVIR_GENERATED_IRQ_HEADER=\"grevir_generated_irq_bindings_mock.hpp\" scratch\interrupt-implementation-gates\mock_event_main.cpp build\irq-msvc\event\grevir_generated_irq_bindings_mock.cpp /Febuild\irq-msvc\event\mock_event_firmware.exe
if errorlevel 1 exit /b 17
build\irq-msvc\event\mock_event_firmware.exe
if errorlevel 1 exit /b 18
cl %CXX_FLAGS% /Zs /DGREVIR_IRQ_PROBE=1 scratch\interrupt-implementation-gates\mock_event_dual_record.cpp >build\irq-msvc\event\dual.log 2>&1
if not errorlevel 1 exit /b 19
cl %CXX_FLAGS% /Zs /Ibuild\irq-msvc\event /DGREVIR_TEST_STALE_ROUTE=1 /DGREVIR_GENERATED_IRQ_HEADER=\"grevir_generated_irq_bindings_mock.hpp\" build\irq-msvc\event\grevir_generated_irq_bindings_mock.cpp >build\irq-msvc\event\stale.log 2>&1
if not errorlevel 1 exit /b 20
echo PASS native MSVC on_event activation, dispatch and negative checks
if not exist build\irq-msvc\zero mkdir build\irq-msvc\zero
cl %CXX_FLAGS% /c /DGREVIR_IRQ_PROBE=1 /Fobuild\irq-msvc\zero\mock_zero_record.obj scratch\interrupt-implementation-gates\mock_zero_record.cpp
if errorlevel 1 exit /b 21
"C:\Users\gianni\AppData\Local\Programs\Python\Python313\python.exe" -B grevir-core\tools\grevir_irqgen\__main__.py plan --object build\irq-msvc\zero\mock_zero_record.obj --out-dir build\irq-msvc\zero --attempt native-msvc-zero --backend mock --target mock_mcu --board mock_board --compiler scratch_compiler
if errorlevel 1 exit /b 22
"C:\Users\gianni\AppData\Local\Programs\Python\Python313\python.exe" -B grevir-core\tools\grevir_irqgen\__main__.py emit --out-dir build\irq-msvc\zero --attempt native-msvc-zero --backend mock --compiler scratch_compiler --application-header scratch/interrupt-implementation-gates/mock_app_base.hpp
if errorlevel 1 exit /b 23
cl %CXX_FLAGS% /Zs /Ibuild\irq-msvc\zero /DGREVIR_TEST_STRICT_ADDED_EVENT=1 /DGREVIR_GENERATED_IRQ_HEADER=\"grevir_generated_irq_bindings_mock.hpp\" build\irq-msvc\zero\grevir_generated_irq_bindings_mock.cpp >build\irq-msvc\zero\stale_demand.log 2>&1
if not errorlevel 1 exit /b 24
findstr /C:"GREVIR_IRQ_STALE_DEMAND_SET" build\irq-msvc\zero\stale_demand.log >nul
if errorlevel 1 exit /b 25
cl %CXX_FLAGS% /Zs /DGREVIR_IRQ_PROBE=1 /DGREVIR_TEST_CAPACITY_VALUE=2048 scratch\interrupt-implementation-gates\mock_deferred_two_record.cpp
if errorlevel 1 exit /b 26
cl %CXX_FLAGS% /Zs /DGREVIR_IRQ_PROBE=1 /DGREVIR_TEST_CAPACITY_VALUE=65537UL scratch\interrupt-implementation-gates\mock_deferred_two_record.cpp >build\irq-msvc\zero\invalid_capacity.log 2>&1
if not errorlevel 1 exit /b 27
findstr /C:"GREVIR_EVENT_CAPACITY_OUT_OF_RANGE" build\irq-msvc\zero\invalid_capacity.log >nul
if errorlevel 1 exit /b 28
echo PASS native MSVC strict-demand and capacity bounds
if not exist build\irq-msvc\stream mkdir build\irq-msvc\stream
cl %CXX_FLAGS% /c /DGREVIR_IRQ_PROBE=1 /DGREVIR_TEST_STREAM_B=1 /Fobuild\irq-msvc\stream\probe.obj scratch\interrupt-implementation-gates\mock_deferred_two_record.cpp
if errorlevel 1 exit /b 29
"C:\Users\gianni\AppData\Local\Programs\Python\Python313\python.exe" -B grevir-core\tools\grevir_irqgen\__main__.py plan --object build\irq-msvc\stream\probe.obj --out-dir build\irq-msvc\stream --attempt native-msvc-stream --backend mock --target mock_mcu --board mock_board --compiler scratch_compiler
if errorlevel 1 exit /b 30
"C:\Users\gianni\AppData\Local\Programs\Python\Python313\python.exe" -B grevir-core\tools\grevir_irqgen\__main__.py emit --out-dir build\irq-msvc\stream --attempt native-msvc-stream --backend mock --compiler scratch_compiler --application-header scratch/interrupt-implementation-gates/mock_deferred_two_app.hpp
if errorlevel 1 exit /b 31
cl %CXX_FLAGS% /Ibuild\irq-msvc\stream /DGREVIR_TEST_STREAM_B=1 /DGREVIR_GENERATED_IRQ_HEADER=\"grevir_generated_irq_bindings_mock.hpp\" scratch\interrupt-implementation-gates\mock_deferred_two_main.cpp build\irq-msvc\stream\grevir_generated_irq_bindings_mock.cpp /Febuild\irq-msvc\stream\firmware.exe
if errorlevel 1 exit /b 32
build\irq-msvc\stream\firmware.exe
if errorlevel 1 exit /b 33
cl %CXX_FLAGS% /Zs /Ibuild\irq-msvc\stream /DGREVIR_GENERATED_IRQ_HEADER=\"grevir_generated_irq_bindings_mock.hpp\" build\irq-msvc\stream\grevir_generated_irq_bindings_mock.cpp >build\irq-msvc\stream\stale.log 2>&1
if not errorlevel 1 exit /b 34
findstr /C:"GREVIR_IRQ_STALE_DEMAND_SET" build\irq-msvc\stream\stale.log >nul
if errorlevel 1 exit /b 35
echo PASS native MSVC stream binding, dispatch and strict route mismatch
exit /b 0
