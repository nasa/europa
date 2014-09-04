find_package(Java REQUIRED)
include(UseJava RESULT_VARIABLE java_included)
if(NOT java_included)
  message(FATAL_ERROR "Failed to include UseJava")
endif(NOT java_included)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/temp)

set(jarclasses "")
set(libjars   lib/bsh-2.0b4.jar
  lib/jcommon-1.0.16.jar
  lib/jfreechart-1.0.13.jar
  lib/egantt/egantt-core.jar
  lib/egantt/egantt-ext.jar
  lib/egantt/egantt-resources.jar)

foreach(jar ${libjars})
  set(classes "")
  unjar(${jar}_unjarred ${CMAKE_CURRENT_SOURCE_DIR}/${jar} temp classes)
#  message(STATUS ${classes})
  list(APPEND jarclasses ${classes})
endforeach(jar)

add_jar(PSUISwing
  INCLUDE_JARS
  ${libjars}
  ${CMAKE_CURRENT_BINARY_DIR}/../PSEngine/PSEngine.jar
  SOURCES
  #${jarclasses}
  source/org/ops/ui/beanshell/swing/BeanShellView.java
  source/org/ops/ui/editor/model/OutlineNode.java
  source/org/ops/ui/editor/model/OutlineNodeType.java
  source/org/ops/ui/editor/swing/anml/AnmlInterpreter.java
  source/org/ops/ui/editor/swing/anml/AnmlTokenMarker.java
  source/org/ops/ui/editor/swing/ash/AshConsole.java
  source/org/ops/ui/editor/swing/ash/AshInterpreter.java
  source/org/ops/ui/editor/swing/ash/AshTokenMarker.java
  source/org/ops/ui/editor/swing/ash/ConsoleDefaults.java
  source/org/ops/ui/editor/swing/ash/ConsolePainter.java
  source/org/ops/ui/editor/swing/ash/DefaultInputHandler.java
  source/org/ops/ui/editor/swing/ash/DocumentOutputStream.java
  source/org/ops/ui/editor/swing/ash/DocumentWriter.java
  source/org/ops/ui/editor/swing/ash/InputHandler.java
  source/org/ops/ui/editor/swing/ash/KeywordMap.java
  source/org/ops/ui/editor/swing/ash/SyntaxDocument.java
  source/org/ops/ui/editor/swing/ash/SyntaxStyle.java
  source/org/ops/ui/editor/swing/ash/SyntaxUtilities.java
  source/org/ops/ui/editor/swing/ash/TextUtilities.java
  source/org/ops/ui/editor/swing/ash/Token.java
  source/org/ops/ui/editor/swing/ash/TokenMarker.java
  source/org/ops/ui/editor/swing/nddl/NddlAshInterpreter.java
  source/org/ops/ui/editor/swing/nddl/NddlTokenMarker.java
  source/org/ops/ui/filemanager/model/AstNode.java
  source/org/ops/ui/filemanager/model/AstNodeTypes.java
  source/org/ops/ui/filemanager/model/ErrorRecord.java
  source/org/ops/ui/filemanager/model/FileModel.java
  source/org/ops/ui/gantt/model/EuropaGanttActivity.java
  source/org/ops/ui/gantt/model/EuropaGanttModel.java
  source/org/ops/ui/gantt/model/EuropaGanttResource.java
  source/org/ops/ui/gantt/model/IGanttActivity.java
  source/org/ops/ui/gantt/model/IGanttModel.java
  source/org/ops/ui/gantt/model/IGanttResource.java
  source/org/ops/ui/gantt/model/PSGanttActivity.java
  source/org/ops/ui/gantt/model/PSGanttActivityImpl.java
  source/org/ops/ui/gantt/model/PSGanttModel.java
  source/org/ops/ui/gantt/model/PSGanttPSEModel.java
  source/org/ops/ui/gantt/swing/GanttView.java
  source/org/ops/ui/gantt/swing/LinePanel.java
  source/org/ops/ui/gantt/swing/PSEGantt.java
  source/org/ops/ui/gantt/swing/PSGantt.java
  source/org/ops/ui/gantt/swing/PSGanttColorProvider.java
  source/org/ops/ui/gantt/swing/ResourcePanel.java
  source/org/ops/ui/gantt/swing/TimelinePanel.java
  source/org/ops/ui/gantt/swing/TokenColor.java
  source/org/ops/ui/gantt/swing/TokenWidget.java
  source/org/ops/ui/main/swing/EuropaInternalFrame.java
  source/org/ops/ui/main/swing/FileNameExtensionFilter.java
  source/org/ops/ui/main/swing/PSComponent.java
  source/org/ops/ui/main/swing/PSComponentBase.java
  source/org/ops/ui/main/swing/PSDesktop.java
  source/org/ops/ui/main/swing/PSMouseListener.java
  source/org/ops/ui/mouse/swing/ActionDetailsPanel.java
  source/org/ops/ui/mouse/swing/ActionViolationsPanel.java
  source/org/ops/ui/mouse/swing/MouseListenerPanel.java
  source/org/ops/ui/rchart/model/PSResourceChartModel.java
  source/org/ops/ui/rchart/model/PSResourceChartPSEModel.java
  source/org/ops/ui/rchart/swing/PSJFreeResourceChart.java
  source/org/ops/ui/rchart/swing/PSResourceChart.java
  source/org/ops/ui/schemabrowser/model/SchemaModel.java
  source/org/ops/ui/schemabrowser/model/SchemaModelImpl.java
  source/org/ops/ui/schemabrowser/model/SchemaNode.java
  source/org/ops/ui/schemabrowser/model/SchemaSolverModel.java
  source/org/ops/ui/schemabrowser/swing/SchemaNodeCellRenderer.java
  source/org/ops/ui/schemabrowser/swing/SchemaTreeModel.java
  source/org/ops/ui/schemabrowser/swing/SchemaView.java
  source/org/ops/ui/solver/model/SolverAdapter.java
  source/org/ops/ui/solver/model/SolverListener.java
  source/org/ops/ui/solver/model/SolverModel.java
  source/org/ops/ui/solver/model/StepStatisticsRecord.java
  source/org/ops/ui/solver/model/TimeFormatHelper.java
  source/org/ops/ui/solver/swing/ConsoleView.java
  source/org/ops/ui/solver/swing/OpenDecisionsPanel.java
  source/org/ops/ui/solver/swing/OpenDecisionsView.java
  source/org/ops/ui/solver/swing/PSSolverDialog.java
  source/org/ops/ui/solver/swing/PSSolverDialogListener.java
  source/org/ops/ui/solver/swing/PSSolverView.java
  source/org/ops/ui/solver/swing/SolverChartPanel.java
  source/org/ops/ui/utils/swing/Util.java
  source/org/ops/ui/utils/Utilities.java
  MANIFEST
  ${CMAKE_CURRENT_SOURCE_DIR}/META-INF/MANIFEST.SWING
  )
# add_dependencies(PSUISwing ${jarclasses})
# foreach(f ${jarclasses})
#   add_custom_command(TARGET PSUISwing
#     POST_BUILD #?
#     COMMAND
#     ${Java_JAR_EXECUTABLE} -uf PSUISwing.jar -C ${CMAKE_CURRENT_BINARY_DIR}/temp ${f}
#     #WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/temp
#     )
# endforeach(f)
# add_custom_command(TARGET PSUISwing
#   POST_BUILD #?
#   COMMAND
#   ${Java_JAR_EXECUTABLE} -uf PSUISwing.jar -C ${CMAKE_CURRENT_BINARY_DIR}/temp ${jarclasses}
#   #WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/temp
#   )
#string(REPLACE ";" "\n" classes_with_newlines ${jarclasses})
foreach(f ${jarclasses})
  file(APPEND PSEngineAdditionalClasses "${f}\n")
endforeach(f)
#file(WRITE PSEngineAdditionalClasses ${jarclasses})
add_custom_command(TARGET PSUISwing
  POST_BUILD #?
  COMMAND
  ${Java_JAR_EXECUTABLE} -uf PSUISwing.jar -C ${CMAKE_CURRENT_BINARY_DIR}/temp .
  #WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/temp
)

install_jar(PSUISwing ${EUROPA_ROOT}/dist/europa)