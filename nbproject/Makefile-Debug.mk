#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Pendulum.o \
	${OBJECTDIR}/Pendulum_CommandLine.o \
	${OBJECTDIR}/Pendulum_File.o \
	${OBJECTDIR}/Pendulum_MailBox.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../Antik/dist/Debug/GNU-Linux/libantik.a -lpthread -lboost_system -lboost_program_options -lboost_filesystem `pkg-config --libs openssl` `pkg-config --libs libcurl`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pendulum

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pendulum: ../Antik/dist/Debug/GNU-Linux/libantik.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pendulum: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pendulum ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/Pendulum.o: Pendulum.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Antik/include -I../Antik/classes/implementation `pkg-config --cflags openssl` `pkg-config --cflags libcurl` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Pendulum.o Pendulum.cpp

${OBJECTDIR}/Pendulum_CommandLine.o: Pendulum_CommandLine.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Antik/include -I../Antik/classes/implementation `pkg-config --cflags openssl` `pkg-config --cflags libcurl` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Pendulum_CommandLine.o Pendulum_CommandLine.cpp

${OBJECTDIR}/Pendulum_File.o: Pendulum_File.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Antik/include -I../Antik/classes/implementation `pkg-config --cflags openssl` `pkg-config --cflags libcurl` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Pendulum_File.o Pendulum_File.cpp

${OBJECTDIR}/Pendulum_MailBox.o: Pendulum_MailBox.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Antik/include -I../Antik/classes/implementation `pkg-config --cflags openssl` `pkg-config --cflags libcurl` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Pendulum_MailBox.o Pendulum_MailBox.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
