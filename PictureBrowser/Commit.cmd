@ECHO OFF
FOR /F %%i IN ('git rev-parse HEAD') DO SET COMMIT=%%i
ECHO #define GIT_COMMIT "%COMMIT%" > Commit.h