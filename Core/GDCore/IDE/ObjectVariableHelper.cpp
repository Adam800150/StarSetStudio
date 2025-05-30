/*
 * GDevelop Core
 * Copyright 2008-2016 Florian Rival (Florian.Rival@gmail.com). All rights
 * reserved. This project is released under the MIT License.
 */
#include "ObjectVariableHelper.h"

#include "GDCore/IDE/WholeProjectRefactorer.h"
#include "GDCore/Project/EventsBasedObject.h"
#include "GDCore/Project/InitialInstancesContainer.h"
#include "GDCore/Project/Object.h"
#include "GDCore/Project/ObjectGroup.h"
#include "GDCore/Project/ObjectsContainer.h"
#include "GDCore/Project/ObjectsContainersList.h"
#include "GDCore/Project/Variable.h"
#include "GDCore/Project/VariablesContainer.h"
#include "GDCore/String.h"

namespace gd {

void ObjectVariableHelper::FillAnyVariableBetweenObjects(
    gd::ObjectsContainer &globalObjectsContainer,
    gd::ObjectsContainer &objectsContainer,
    const gd::ObjectGroup &objectGroup) {
  const auto &objectNames = objectGroup.GetAllObjectsNames();
  for (const gd::String &sourceObjectName : objectNames) {
    const bool hasObject = objectsContainer.HasObjectNamed(sourceObjectName);
    if (!hasObject &&
        !globalObjectsContainer.HasObjectNamed(sourceObjectName)) {
      continue;
    }
    const auto &sourceObject =
        hasObject ? objectsContainer.GetObject(sourceObjectName)
                  : globalObjectsContainer.GetObject(sourceObjectName);
    const auto &sourceVariablesContainer = sourceObject.GetVariables();

    for (const gd::String &destinationObjectName : objectNames) {
      if (sourceObjectName == destinationObjectName) {
        continue;
      }
      const bool hasObject =
          objectsContainer.HasObjectNamed(destinationObjectName);
      if (!hasObject &&
          !globalObjectsContainer.HasObjectNamed(destinationObjectName)) {
        continue;
      }
      auto &destinationObject =
          hasObject ? objectsContainer.GetObject(destinationObjectName)
                    : globalObjectsContainer.GetObject(destinationObjectName);
      auto &destinationVariablesContainer = destinationObject.GetVariables();

      for (std::size_t sourceVariableIndex = 0;
           sourceVariableIndex < sourceVariablesContainer.Count();
           ++sourceVariableIndex) {
        auto &sourceVariable =
            sourceVariablesContainer.Get(sourceVariableIndex);
        const auto &variableName =
            sourceVariablesContainer.GetNameAt(sourceVariableIndex);

        if (!destinationVariablesContainer.Has(variableName)) {
          destinationVariablesContainer.Insert(
              variableName, sourceVariable,
              destinationVariablesContainer.Count());
        }
      }
    }
  }
}

gd::VariablesContainer ObjectVariableHelper::MergeVariableContainers(
    const gd::ObjectsContainersList &objectsContainersList,
    const gd::ObjectGroup &objectGroup) {
  gd::VariablesContainer mergedVariablesContainer;

  const auto &objectNames = objectGroup.GetAllObjectsNames();
  std::size_t objectIndex = 0;
  bool isFirstObjectFound = false;
  for (; objectIndex < objectNames.size() && !isFirstObjectFound;
       objectIndex++) {
    const gd::String &objectName = objectNames[objectIndex];
    if (!objectsContainersList.HasObjectOrGroupNamed(objectName)) {
      continue;
    }
    isFirstObjectFound = true;
    mergedVariablesContainer =
        *objectsContainersList.GetObjectOrGroupVariablesContainer(objectName);
  }
  for (; objectIndex < objectNames.size(); objectIndex++) {
    const gd::String &objectName = objectNames[objectIndex];
    if (!objectsContainersList.HasObjectOrGroupNamed(objectName)) {
      continue;
    }
    const auto &variablesContainer =
        *objectsContainersList.GetObjectOrGroupVariablesContainer(objectName);

    for (std::size_t variableIndex = 0;
         variableIndex < mergedVariablesContainer.Count(); ++variableIndex) {
      auto &mergedVariable = mergedVariablesContainer.Get(variableIndex);
      const auto &variableName =
          mergedVariablesContainer.GetNameAt(variableIndex);

      if (variablesContainer.Has(variableName)) {
        auto &variable = variablesContainer.Get(variableName);
        if (mergedVariable.GetType() != variable.GetType()) {
          mergedVariable.CastTo(gd::Variable::Type::MixedTypes);
        } else if (mergedVariable != variable) {
          mergedVariable.MarkAsMixedValues();
        }
      } else {
        mergedVariablesContainer.Remove(variableName);
        variableIndex--;
      }
    }
  }
  return mergedVariablesContainer;
}

void ObjectVariableHelper::FillMissingGroupVariablesToObjects(
    gd::ObjectsContainer &globalObjectsContainer,
    gd::ObjectsContainer &objectsContainer, const gd::ObjectGroup &objectGroup,
    const gd::SerializerElement &originalSerializedVariables) {
  gd::VariablesContainer groupVariablesContainer;
  groupVariablesContainer.UnserializeFrom(originalSerializedVariables);
  // Add missing variables to objects added in the group.
  for (const gd::String &objectName : objectGroup.GetAllObjectsNames()) {
    const bool hasObject = objectsContainer.HasObjectNamed(objectName);
    if (!hasObject && !globalObjectsContainer.HasObjectNamed(objectName)) {
      continue;
    }
    auto &object = hasObject ? objectsContainer.GetObject(objectName)
                             : globalObjectsContainer.GetObject(objectName);
    auto &variablesContainer = object.GetVariables();
    for (std::size_t variableIndex = 0;
         variableIndex < groupVariablesContainer.Count(); ++variableIndex) {
      auto &groupVariable = groupVariablesContainer.Get(variableIndex);
      const auto &variableName =
          groupVariablesContainer.GetNameAt(variableIndex);

      if (!variablesContainer.Has(variableName)) {
        variablesContainer.Insert(variableName, groupVariable,
                                  variablesContainer.Count());
      }
    }
  }
};

// TODO Handle position changes for group variables.
// We could try to change the order of object variables in a way that the next
// call to MergeVariableContainers rebuild them in the same order.
void ObjectVariableHelper::ApplyChangesToObjects(
    gd::ObjectsContainer &globalObjectsContainer,
    gd::ObjectsContainer &objectsContainer,
    const gd::VariablesContainer &groupVariablesContainer,
    const gd::ObjectGroup &objectGroup,
    const gd::VariablesChangeset &changeset) {
  for (const gd::String &objectName : objectGroup.GetAllObjectsNames()) {
    const bool hasObject = objectsContainer.HasObjectNamed(objectName);
    if (!hasObject && !globalObjectsContainer.HasObjectNamed(objectName)) {
      continue;
    }
    auto &object = hasObject ? objectsContainer.GetObject(objectName)
                             : globalObjectsContainer.GetObject(objectName);
    auto &variablesContainer = object.GetVariables();
    for (const gd::String &variableName : changeset.removedVariableNames) {
      variablesContainer.Remove(variableName);
    }
    for (const gd::String &variableName : changeset.addedVariableNames) {
      if (variablesContainer.Has(variableName)) {
        // It can happens if an object already had the variable but it was not
        // shared by other object of the group.
        continue;
      }
      variablesContainer.Insert(variableName,
                                groupVariablesContainer.Get(variableName),
                                variablesContainer.Count());
    }
    // TODO Check what happens if 2 variables exchange their names.
    for (const auto &pair : changeset.oldToNewVariableNames) {
      const gd::String &oldVariableName = pair.first;
      const gd::String &newVariableName = pair.second;
      if (variablesContainer.Has(newVariableName)) {
        // It can happens if an object already had the variable but it was not
        // shared by other object of the group.
        variablesContainer.Remove(oldVariableName);
      } else {
        variablesContainer.Rename(oldVariableName, newVariableName);
      }
    }
    // Apply type and value changes
    for (const gd::String &variableName : changeset.valueChangedVariableNames) {
      size_t index = variablesContainer.GetPosition(variableName);
      variablesContainer.Remove(variableName);
      variablesContainer.Insert(
          variableName, groupVariablesContainer.Get(variableName), index);
    }
  }
}

void ObjectVariableHelper::ApplyChangesToObjectInstances(
    gd::VariablesContainer &objectVariablesContainer,
    gd::InitialInstancesContainer &initialInstancesContainer,
    const gd::String &objectName, const gd::VariablesChangeset &changeset) {
  initialInstancesContainer.IterateOverInstances(
      [&objectVariablesContainer, &objectName,
       &changeset](gd::InitialInstance &instance) {
        if (instance.GetObjectName() == objectName) {
          auto &destinationVariablesContainer = instance.GetVariables();
          for (const gd::String &variableName :
               changeset.removedVariableNames) {
            destinationVariablesContainer.Remove(variableName);
          }
          for (const gd::String &variableName : changeset.addedVariableNames) {
            // Instance variables may already exist with another type.
            if (destinationVariablesContainer.Has(variableName) &&
                destinationVariablesContainer.Get(variableName).GetType() !=
                    objectVariablesContainer.Get(variableName).GetType()) {
              destinationVariablesContainer.Remove(variableName);
            }
          }
          // TODO Check what happens if 2 variables exchange their names.
          for (const auto &pair : changeset.oldToNewVariableNames) {
            const gd::String &oldVariableName = pair.first;
            const gd::String &newVariableName = pair.second;
            if (destinationVariablesContainer.Has(newVariableName)) {
              // It can happens if an instance already had the variable.
              destinationVariablesContainer.Remove(oldVariableName);
            } else {
              destinationVariablesContainer.Rename(oldVariableName,
                                                   newVariableName);
            }
          }
          // Apply type changes
          for (const gd::String &variableName :
               changeset.valueChangedVariableNames) {
            if (destinationVariablesContainer.Has(variableName) &&
                destinationVariablesContainer.Get(variableName).GetType() !=
                    objectVariablesContainer.Get(variableName).GetType()) {
              destinationVariablesContainer.Remove(variableName);
            }
          }
        }
        return false;
      });
}

void ObjectVariableHelper::ApplyChangesToVariants(
    gd::EventsBasedObject &eventsBasedObject, const gd::String &objectName,
    const gd::VariablesChangeset &changeset) {
  auto &defaultVariablesContainer = eventsBasedObject.GetDefaultVariant()
                                        .GetObjects()
                                        .GetObject(objectName)
                                        .GetVariables();
  for (auto &variant : eventsBasedObject.GetVariants().GetInternalVector()) {
    if (!variant->GetObjects().HasObjectNamed(objectName)) {
      continue;
    }
    auto &object = variant->GetObjects().GetObject(objectName);
    auto &variablesContainer = object.GetVariables();

    for (const gd::String &variableName : changeset.removedVariableNames) {
      variablesContainer.Remove(variableName);
    }
    for (const gd::String &variableName : changeset.addedVariableNames) {
      if (variablesContainer.Has(variableName)) {
        // It can happens if a child-object already had the variable but it was
        // missing in other variant child-object.
        continue;
      }
      variablesContainer.Insert(variableName,
                                defaultVariablesContainer.Get(variableName),
                                variablesContainer.Count());
    }
    // TODO Check what happens if 2 variables exchange their names.
    for (const auto &pair : changeset.oldToNewVariableNames) {
      const gd::String &oldVariableName = pair.first;
      const gd::String &newVariableName = pair.second;
      if (variablesContainer.Has(newVariableName)) {
        // It can happens if a child-object already had the variable but it was
        // missing in other variant child-object.
        variablesContainer.Remove(oldVariableName);
      } else {
        variablesContainer.Rename(oldVariableName, newVariableName);
      }
    }
    // Apply type changes
    for (const gd::String &variableName : changeset.valueChangedVariableNames) {
      size_t index = variablesContainer.GetPosition(variableName);

      if (variablesContainer.Has(variableName) &&
          variablesContainer.Get(variableName).GetType() !=
              defaultVariablesContainer.Get(variableName).GetType()) {
        variablesContainer.Remove(variableName);
        variablesContainer.Insert(
            variableName, defaultVariablesContainer.Get(variableName), index);
      }
    }

    gd::ObjectVariableHelper::ApplyChangesToObjectInstances(
        variablesContainer, variant->GetInitialInstances(), objectName,
        changeset);
  }
}

} // namespace gd
