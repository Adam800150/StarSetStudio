// @flow
import * as React from 'react';
import { Trans } from '@lingui/macro';
import VariablesEditorDialog from './VariablesEditorDialog';
import { type HotReloadPreviewButtonProps } from '../HotReload/HotReloadPreviewButton';
import EventsRootVariablesFinder from '../Utils/EventsRootVariablesFinder';
import { ProjectScopedContainersAccessor } from '../InstructionOrExpression/EventsScope';

type Props = {|
  open: boolean,
  project: gdProject,
  onApply: (selectedVariableName: string | null) => void,
  onCancel: () => void,
  hotReloadPreviewButtonProps: HotReloadPreviewButtonProps | null,
  initiallySelectedVariableName?: string,
  shouldCreateInitiallySelectedVariable?: boolean,
  isListLocked: boolean,
|};

const GlobalVariablesDialog = ({
  project,
  open,
  onCancel,
  onApply,
  hotReloadPreviewButtonProps,
  initiallySelectedVariableName,
  shouldCreateInitiallySelectedVariable,
  isListLocked,
}: Props) => {
  const onComputeAllVariableNames = React.useCallback(
    () =>
      EventsRootVariablesFinder.findAllGlobalVariables(
        project.getCurrentPlatform(),
        project
      ),
    [project]
  );

  const tabs = React.useMemo(
    () => [
      {
        id: 'global-variables',
        label: <Trans>Global variables</Trans>,
        variablesContainer: project.getVariables(),
        emptyPlaceholderTitle: <Trans>Add your first global variable</Trans>,
        emptyPlaceholderDescription: (
          <Trans>
            These variables hold additional information on a project.
          </Trans>
        ),
        onComputeAllVariableNames,
      },
    ],
    [project, onComputeAllVariableNames]
  );

  const projectScopedContainersAccessor = React.useMemo(
    () =>
      new ProjectScopedContainersAccessor({
        project,
      }),
    [project]
  );

  return (
    <VariablesEditorDialog
      project={project}
      projectScopedContainersAccessor={projectScopedContainersAccessor}
      open={open}
      onCancel={onCancel}
      onApply={onApply}
      title={<Trans>Global variables</Trans>}
      tabs={tabs}
      initiallySelectedVariableName={initiallySelectedVariableName}
      shouldCreateInitiallySelectedVariable={
        shouldCreateInitiallySelectedVariable
      }
      helpPagePath={'/all-features/variables/global-variables'}
      hotReloadPreviewButtonProps={hotReloadPreviewButtonProps}
      id="global-variables-dialog"
      isListLocked={isListLocked}
    />
  );
};

export default GlobalVariablesDialog;
