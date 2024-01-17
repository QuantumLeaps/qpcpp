#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-dbg.mk)" "nbproject/Makefile-local-dbg.mk"
include nbproject/Makefile-local-dbg.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=dbg
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/xc32.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/xc32.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../../../src/qf/qep_hsm.cpp ../../../../../src/qf/qep_msm.cpp ../../../../../src/qf/qf_act.cpp ../../../../../src/qf/qf_actq.cpp ../../../../../src/qf/qf_defer.cpp ../../../../../src/qf/qf_dyn.cpp ../../../../../src/qf/qf_mem.cpp ../../../../../src/qf/qf_ps.cpp ../../../../../src/qf/qf_qact.cpp ../../../../../src/qf/qf_qeq.cpp ../../../../../src/qf/qf_qmact.cpp ../../../../../src/qf/qf_time.cpp ../../../../../src/qk/qk.cpp ../../../../../ports/pic32/qk/xc32/qk_port.cpp ../../../../../src/qs/qstamp.cpp ../../main.cpp ../../philo.cpp ../../table.cpp bsp.cpp

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/820665871/qep_hsm.o ${OBJECTDIR}/_ext/820665871/qep_msm.o ${OBJECTDIR}/_ext/820665871/qf_act.o ${OBJECTDIR}/_ext/820665871/qf_actq.o ${OBJECTDIR}/_ext/820665871/qf_defer.o ${OBJECTDIR}/_ext/820665871/qf_dyn.o ${OBJECTDIR}/_ext/820665871/qf_mem.o ${OBJECTDIR}/_ext/820665871/qf_ps.o ${OBJECTDIR}/_ext/820665871/qf_qact.o ${OBJECTDIR}/_ext/820665871/qf_qeq.o ${OBJECTDIR}/_ext/820665871/qf_qmact.o ${OBJECTDIR}/_ext/820665871/qf_time.o ${OBJECTDIR}/_ext/820665876/qk.o ${OBJECTDIR}/_ext/1627782163/qk_port.o ${OBJECTDIR}/_ext/820665884/qstamp.o ${OBJECTDIR}/_ext/43898991/main.o ${OBJECTDIR}/_ext/43898991/philo.o ${OBJECTDIR}/_ext/43898991/table.o ${OBJECTDIR}/bsp.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/820665871/qep_hsm.o.d ${OBJECTDIR}/_ext/820665871/qep_msm.o.d ${OBJECTDIR}/_ext/820665871/qf_act.o.d ${OBJECTDIR}/_ext/820665871/qf_actq.o.d ${OBJECTDIR}/_ext/820665871/qf_defer.o.d ${OBJECTDIR}/_ext/820665871/qf_dyn.o.d ${OBJECTDIR}/_ext/820665871/qf_mem.o.d ${OBJECTDIR}/_ext/820665871/qf_ps.o.d ${OBJECTDIR}/_ext/820665871/qf_qact.o.d ${OBJECTDIR}/_ext/820665871/qf_qeq.o.d ${OBJECTDIR}/_ext/820665871/qf_qmact.o.d ${OBJECTDIR}/_ext/820665871/qf_time.o.d ${OBJECTDIR}/_ext/820665876/qk.o.d ${OBJECTDIR}/_ext/1627782163/qk_port.o.d ${OBJECTDIR}/_ext/820665884/qstamp.o.d ${OBJECTDIR}/_ext/43898991/main.o.d ${OBJECTDIR}/_ext/43898991/philo.o.d ${OBJECTDIR}/_ext/43898991/table.o.d ${OBJECTDIR}/bsp.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/820665871/qep_hsm.o ${OBJECTDIR}/_ext/820665871/qep_msm.o ${OBJECTDIR}/_ext/820665871/qf_act.o ${OBJECTDIR}/_ext/820665871/qf_actq.o ${OBJECTDIR}/_ext/820665871/qf_defer.o ${OBJECTDIR}/_ext/820665871/qf_dyn.o ${OBJECTDIR}/_ext/820665871/qf_mem.o ${OBJECTDIR}/_ext/820665871/qf_ps.o ${OBJECTDIR}/_ext/820665871/qf_qact.o ${OBJECTDIR}/_ext/820665871/qf_qeq.o ${OBJECTDIR}/_ext/820665871/qf_qmact.o ${OBJECTDIR}/_ext/820665871/qf_time.o ${OBJECTDIR}/_ext/820665876/qk.o ${OBJECTDIR}/_ext/1627782163/qk_port.o ${OBJECTDIR}/_ext/820665884/qstamp.o ${OBJECTDIR}/_ext/43898991/main.o ${OBJECTDIR}/_ext/43898991/philo.o ${OBJECTDIR}/_ext/43898991/table.o ${OBJECTDIR}/bsp.o

# Source Files
SOURCEFILES=../../../../../src/qf/qep_hsm.cpp ../../../../../src/qf/qep_msm.cpp ../../../../../src/qf/qf_act.cpp ../../../../../src/qf/qf_actq.cpp ../../../../../src/qf/qf_defer.cpp ../../../../../src/qf/qf_dyn.cpp ../../../../../src/qf/qf_mem.cpp ../../../../../src/qf/qf_ps.cpp ../../../../../src/qf/qf_qact.cpp ../../../../../src/qf/qf_qeq.cpp ../../../../../src/qf/qf_qmact.cpp ../../../../../src/qf/qf_time.cpp ../../../../../src/qk/qk.cpp ../../../../../ports/pic32/qk/xc32/qk_port.cpp ../../../../../src/qs/qstamp.cpp ../../main.cpp ../../philo.cpp ../../table.cpp bsp.cpp



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-dbg.mk ${DISTDIR}/xc32.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX250F128B
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/820665871/qep_hsm.o: ../../../../../src/qf/qep_hsm.cpp  .generated_files/flags/dbg/81210811278d288637bc9fc915a83dd4d5f779c4 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qep_hsm.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qep_hsm.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qep_hsm.o.d" -o ${OBJECTDIR}/_ext/820665871/qep_hsm.o ../../../../../src/qf/qep_hsm.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qep_msm.o: ../../../../../src/qf/qep_msm.cpp  .generated_files/flags/dbg/5f426310d281fbefdda395d5099e819ca7183909 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qep_msm.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qep_msm.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qep_msm.o.d" -o ${OBJECTDIR}/_ext/820665871/qep_msm.o ../../../../../src/qf/qep_msm.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_act.o: ../../../../../src/qf/qf_act.cpp  .generated_files/flags/dbg/bbfd0f0648549f4abbc1bb84dab5a43435540f73 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_act.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_act.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_act.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_act.o ../../../../../src/qf/qf_act.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_actq.o: ../../../../../src/qf/qf_actq.cpp  .generated_files/flags/dbg/40450d3da67b850856d14286e4fd71cbad967c87 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_actq.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_actq.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_actq.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_actq.o ../../../../../src/qf/qf_actq.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_defer.o: ../../../../../src/qf/qf_defer.cpp  .generated_files/flags/dbg/bc1ac7a01fc5fa34e3a5fbb447a67958e0dd5719 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_defer.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_defer.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_defer.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_defer.o ../../../../../src/qf/qf_defer.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_dyn.o: ../../../../../src/qf/qf_dyn.cpp  .generated_files/flags/dbg/870f5cf3f2bfd76875421bbb0a59e4e1c252b6c .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_dyn.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_dyn.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_dyn.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_dyn.o ../../../../../src/qf/qf_dyn.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_mem.o: ../../../../../src/qf/qf_mem.cpp  .generated_files/flags/dbg/5f5b2da0b1cd86620c31223f572d3cf0d601a2ad .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_mem.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_mem.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_mem.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_mem.o ../../../../../src/qf/qf_mem.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_ps.o: ../../../../../src/qf/qf_ps.cpp  .generated_files/flags/dbg/8f75ddf1de40572f54650fd4f162e606025ebf81 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_ps.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_ps.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_ps.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_ps.o ../../../../../src/qf/qf_ps.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_qact.o: ../../../../../src/qf/qf_qact.cpp  .generated_files/flags/dbg/47511662997f825effe33708c11b763991a0cfb5 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qact.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qact.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_qact.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_qact.o ../../../../../src/qf/qf_qact.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_qeq.o: ../../../../../src/qf/qf_qeq.cpp  .generated_files/flags/dbg/4d2deea56278708dd46eb72074dfb6e41cee8d12 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qeq.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qeq.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_qeq.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_qeq.o ../../../../../src/qf/qf_qeq.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_qmact.o: ../../../../../src/qf/qf_qmact.cpp  .generated_files/flags/dbg/2aa2b41ee77213fed0600e57fa3e40972faed385 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qmact.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qmact.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_qmact.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_qmact.o ../../../../../src/qf/qf_qmact.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_time.o: ../../../../../src/qf/qf_time.cpp  .generated_files/flags/dbg/a956ff76d47d176d71a9de13a427381506ff1372 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_time.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_time.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_time.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_time.o ../../../../../src/qf/qf_time.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665876/qk.o: ../../../../../src/qk/qk.cpp  .generated_files/flags/dbg/a91a9f6e9577cbf8e7180e68fee7c2ff62527cbd .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665876" 
	@${RM} ${OBJECTDIR}/_ext/820665876/qk.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665876/qk.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665876/qk.o.d" -o ${OBJECTDIR}/_ext/820665876/qk.o ../../../../../src/qk/qk.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/1627782163/qk_port.o: ../../../../../ports/pic32/qk/xc32/qk_port.cpp  .generated_files/flags/dbg/cc29aa80606948651a44452435a00ff6a9626064 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1627782163" 
	@${RM} ${OBJECTDIR}/_ext/1627782163/qk_port.o.d 
	@${RM} ${OBJECTDIR}/_ext/1627782163/qk_port.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/1627782163/qk_port.o.d" -o ${OBJECTDIR}/_ext/1627782163/qk_port.o ../../../../../ports/pic32/qk/xc32/qk_port.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665884/qstamp.o: ../../../../../src/qs/qstamp.cpp  .generated_files/flags/dbg/23e0ff1a2f33719fcb373df220e5be9274cc10c2 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665884" 
	@${RM} ${OBJECTDIR}/_ext/820665884/qstamp.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665884/qstamp.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665884/qstamp.o.d" -o ${OBJECTDIR}/_ext/820665884/qstamp.o ../../../../../src/qs/qstamp.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/43898991/main.o: ../../main.cpp  .generated_files/flags/dbg/b98ac871914b5a03c30de2c118a7ef71a18bca85 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/43898991" 
	@${RM} ${OBJECTDIR}/_ext/43898991/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/43898991/main.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/43898991/main.o.d" -o ${OBJECTDIR}/_ext/43898991/main.o ../../main.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/43898991/philo.o: ../../philo.cpp  .generated_files/flags/dbg/20f9934b26f2bd53abca2e44b6b4d51923d788d4 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/43898991" 
	@${RM} ${OBJECTDIR}/_ext/43898991/philo.o.d 
	@${RM} ${OBJECTDIR}/_ext/43898991/philo.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/43898991/philo.o.d" -o ${OBJECTDIR}/_ext/43898991/philo.o ../../philo.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/43898991/table.o: ../../table.cpp  .generated_files/flags/dbg/983c92ca54d665e7657f9212de16bfd8093a60f8 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/43898991" 
	@${RM} ${OBJECTDIR}/_ext/43898991/table.o.d 
	@${RM} ${OBJECTDIR}/_ext/43898991/table.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/43898991/table.o.d" -o ${OBJECTDIR}/_ext/43898991/table.o ../../table.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/bsp.o: bsp.cpp  .generated_files/flags/dbg/80121346fc68e1df7dcff23c708b94f4c53f804c .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bsp.o.d 
	@${RM} ${OBJECTDIR}/bsp.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -fframe-base-loclist  -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/bsp.o.d" -o ${OBJECTDIR}/bsp.o bsp.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
else
${OBJECTDIR}/_ext/820665871/qep_hsm.o: ../../../../../src/qf/qep_hsm.cpp  .generated_files/flags/dbg/88479abe977da16bd8d1a6f239050e6314815b6e .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qep_hsm.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qep_hsm.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qep_hsm.o.d" -o ${OBJECTDIR}/_ext/820665871/qep_hsm.o ../../../../../src/qf/qep_hsm.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qep_msm.o: ../../../../../src/qf/qep_msm.cpp  .generated_files/flags/dbg/ff57abd02c7a97e91305a31c3b5afd4509698ed9 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qep_msm.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qep_msm.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qep_msm.o.d" -o ${OBJECTDIR}/_ext/820665871/qep_msm.o ../../../../../src/qf/qep_msm.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_act.o: ../../../../../src/qf/qf_act.cpp  .generated_files/flags/dbg/64a08f5fa5651eb9a7d40516e59838382f172e15 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_act.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_act.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_act.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_act.o ../../../../../src/qf/qf_act.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_actq.o: ../../../../../src/qf/qf_actq.cpp  .generated_files/flags/dbg/d2cd955259f248289d5a6ce8dcb38209273be464 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_actq.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_actq.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_actq.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_actq.o ../../../../../src/qf/qf_actq.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_defer.o: ../../../../../src/qf/qf_defer.cpp  .generated_files/flags/dbg/cfb22ee7487f7fdb33119a3ab40752eb1376d24a .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_defer.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_defer.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_defer.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_defer.o ../../../../../src/qf/qf_defer.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_dyn.o: ../../../../../src/qf/qf_dyn.cpp  .generated_files/flags/dbg/2cc691290e7660e17a9f48e8e8e84010891eca3c .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_dyn.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_dyn.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_dyn.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_dyn.o ../../../../../src/qf/qf_dyn.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_mem.o: ../../../../../src/qf/qf_mem.cpp  .generated_files/flags/dbg/5013a5719086d195cdf529f0a9d2f28207cdf2d .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_mem.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_mem.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_mem.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_mem.o ../../../../../src/qf/qf_mem.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_ps.o: ../../../../../src/qf/qf_ps.cpp  .generated_files/flags/dbg/da9bce521c886b7af7204bffd73da7e717f5e1f8 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_ps.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_ps.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_ps.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_ps.o ../../../../../src/qf/qf_ps.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_qact.o: ../../../../../src/qf/qf_qact.cpp  .generated_files/flags/dbg/9172668d22e2042a8eddab57365456c345cfd59 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qact.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qact.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_qact.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_qact.o ../../../../../src/qf/qf_qact.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_qeq.o: ../../../../../src/qf/qf_qeq.cpp  .generated_files/flags/dbg/1bc9365a4cb08daa6b1e592e51231a3c6e114eed .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qeq.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qeq.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_qeq.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_qeq.o ../../../../../src/qf/qf_qeq.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_qmact.o: ../../../../../src/qf/qf_qmact.cpp  .generated_files/flags/dbg/86e185b6421b5ec9d6c38ebd684d1864ba7accb .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qmact.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_qmact.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_qmact.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_qmact.o ../../../../../src/qf/qf_qmact.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665871/qf_time.o: ../../../../../src/qf/qf_time.cpp  .generated_files/flags/dbg/9d235860a7ee9417036771f537582e6c87a48b4f .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665871" 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_time.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665871/qf_time.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665871/qf_time.o.d" -o ${OBJECTDIR}/_ext/820665871/qf_time.o ../../../../../src/qf/qf_time.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665876/qk.o: ../../../../../src/qk/qk.cpp  .generated_files/flags/dbg/bca1b415b663960dfd949546a42777800389d1e0 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665876" 
	@${RM} ${OBJECTDIR}/_ext/820665876/qk.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665876/qk.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665876/qk.o.d" -o ${OBJECTDIR}/_ext/820665876/qk.o ../../../../../src/qk/qk.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/1627782163/qk_port.o: ../../../../../ports/pic32/qk/xc32/qk_port.cpp  .generated_files/flags/dbg/d943aa383327b3c07c1ceb268f610f95f3c21e3c .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1627782163" 
	@${RM} ${OBJECTDIR}/_ext/1627782163/qk_port.o.d 
	@${RM} ${OBJECTDIR}/_ext/1627782163/qk_port.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/1627782163/qk_port.o.d" -o ${OBJECTDIR}/_ext/1627782163/qk_port.o ../../../../../ports/pic32/qk/xc32/qk_port.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/820665884/qstamp.o: ../../../../../src/qs/qstamp.cpp  .generated_files/flags/dbg/3583d257c4733a85d2b3f639859a7772d4a4209 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/820665884" 
	@${RM} ${OBJECTDIR}/_ext/820665884/qstamp.o.d 
	@${RM} ${OBJECTDIR}/_ext/820665884/qstamp.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/820665884/qstamp.o.d" -o ${OBJECTDIR}/_ext/820665884/qstamp.o ../../../../../src/qs/qstamp.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/43898991/main.o: ../../main.cpp  .generated_files/flags/dbg/4890aa644d3248d6cbbc10654f95cb0221fb67b7 .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/43898991" 
	@${RM} ${OBJECTDIR}/_ext/43898991/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/43898991/main.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/43898991/main.o.d" -o ${OBJECTDIR}/_ext/43898991/main.o ../../main.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/43898991/philo.o: ../../philo.cpp  .generated_files/flags/dbg/7f7ced91ad0d10eba8e709980bd55c8190a0442a .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/43898991" 
	@${RM} ${OBJECTDIR}/_ext/43898991/philo.o.d 
	@${RM} ${OBJECTDIR}/_ext/43898991/philo.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/43898991/philo.o.d" -o ${OBJECTDIR}/_ext/43898991/philo.o ../../philo.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/_ext/43898991/table.o: ../../table.cpp  .generated_files/flags/dbg/141202700740c3a56356fdca81122959d451cb7c .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/43898991" 
	@${RM} ${OBJECTDIR}/_ext/43898991/table.o.d 
	@${RM} ${OBJECTDIR}/_ext/43898991/table.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/_ext/43898991/table.o.d" -o ${OBJECTDIR}/_ext/43898991/table.o ../../table.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
${OBJECTDIR}/bsp.o: bsp.cpp  .generated_files/flags/dbg/8126cd8e88b19ab4bf9b0b624feda0a89b9fd21d .generated_files/flags/dbg/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bsp.o.d 
	@${RM} ${OBJECTDIR}/bsp.o 
	${MP_CPPC} $(MP_EXTRA_CC_PRE)  -g -x c++ -c -mprocessor=$(MP_PROCESSOR_OPTION)  -fno-rtti -fno-exceptions -fno-check-new -fno-enforce-eh-specs -I"../.." -I"../../../../../include" -I"../../../../../src" -I"../../../../../ports/pic32/qk/xc32" -MP -MMD -MF "${OBJECTDIR}/bsp.o.d" -o ${OBJECTDIR}/bsp.o bsp.cpp   -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)    
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/xc32.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CPPC} $(MP_EXTRA_LD_PRE) -g -mdebugger -D__MPLAB_DEBUGGER_PK3=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/xc32.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)   -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC00490:0x1FC00BEF  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=__MPLAB_DEBUGGER_PK3=1,--defsym=_min_heap_size=0,--no-code-in-dinit,--no-dinit-in-serial-mem,--report-mem,--warn-section-align,--memorysummary,${DISTDIR}/memoryfile.xml 
	
else
${DISTDIR}/xc32.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CPPC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/xc32.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_dbg=$(CND_CONF)  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=0,--no-code-in-dinit,--no-dinit-in-serial-mem,--report-mem,--warn-section-align,--memorysummary,${DISTDIR}/memoryfile.xml 
	${MP_CC_DIR}\\xc32-bin2hex ${DISTDIR}/xc32.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
