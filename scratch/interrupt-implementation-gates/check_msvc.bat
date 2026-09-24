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
exit /b 0
