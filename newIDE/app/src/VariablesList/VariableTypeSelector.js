// @flow
import * as React from 'react';
import { t } from '@lingui/macro';

import { type MessageDescriptor } from '../Utils/i18n/MessageDescriptor.flow';
import SelectField from '../UI/SelectField';
import SelectOption from '../UI/SelectOption';
import VariableStringIcon from './Icons/VariableStringIcon';
import VariableNumberIcon from './Icons/VariableNumberIcon';
import VariableBooleanIcon from './Icons/VariableBooleanIcon';
import VariableArrayIcon from './Icons/VariableArrayIcon';
import VariableStructureIcon from './Icons/VariableStructureIcon';
import VariableMixedTypesIcon from '../UI/CustomSvgIcons/Cross';
import WarningIcon from '../UI/CustomSvgIcons/Warning';
import { Line, Spacer } from '../UI/Grid';
import GDevelopThemeContext from '../UI/Theme/GDevelopThemeContext';
import Tooltip from '@material-ui/core/Tooltip';
const gd = global.gd;

type Props = {|
  variableType: Variable_Type,
  onChange: (newVariableType: string, nodeId: string) => void,
  nodeId: string,
  isHighlighted?: boolean,
  readOnlyWithIcon?: boolean,
  id?: string,
  errorMessage: MessageDescriptor | null,
  disabled?: boolean,
|};

let options;
let variableTypeToIcon;
let variableTypeToString;

const getOptions = () => {
  if (!options) {
    options = [
      <SelectOption key="string" label={t`Text`} value={gd.Variable.String} />,
      <SelectOption
        key="number"
        label={t`Number`}
        value={gd.Variable.Number}
      />,
      <SelectOption
        key="boolean"
        label={t`Boolean`}
        value={gd.Variable.Boolean}
      />,
      <SelectOption key="array" label={t`Array`} value={gd.Variable.Array} />,
      <SelectOption
        key="structure"
        label={t`Structure`}
        value={gd.Variable.Structure}
      />,
    ];
  }
  return options;
};

export const getVariableTypeToIcon = (): { [Variable_Type]: any } => {
  if (!variableTypeToIcon) {
    variableTypeToIcon = {
      [gd.Variable.MixedTypes]: VariableMixedTypesIcon,
      [gd.Variable.String]: VariableStringIcon,
      [gd.Variable.Number]: VariableNumberIcon,
      [gd.Variable.Boolean]: VariableBooleanIcon,
      [gd.Variable.Array]: VariableArrayIcon,
      [gd.Variable.Structure]: VariableStructureIcon,
    };
  }
  return variableTypeToIcon;
};

const getVariableTypeToString = () => {
  if (!variableTypeToString) {
    variableTypeToString = {
      [gd.Variable.String]: 'string',
      [gd.Variable.Number]: 'number',
      [gd.Variable.Boolean]: 'boolean',
      [gd.Variable.Array]: 'array',
      [gd.Variable.Structure]: 'structure',
    };
  }
  return variableTypeToString;
};

const VariableTypeSelector = React.memo<Props>((props: Props) => {
  const gdevelopTheme = React.useContext(GDevelopThemeContext);
  const Icon = getVariableTypeToIcon()[props.variableType];

  return (
    <Line alignItems="center" noMargin>
      {props.errorMessage ? (
        <Tooltip title={props.errorMessage}>
          <WarningIcon
            fontSize="small"
            htmlColor={gdevelopTheme.message.warning}
          />
        </Tooltip>
      ) : (
        <Icon
          fontSize="small"
          htmlColor={
            props.isHighlighted
              ? gdevelopTheme.message.selectedTextColor
              : undefined
          }
        />
      )}
      {!props.readOnlyWithIcon && (
        <>
          <Spacer />
          <SelectField
            value={props.variableType}
            translatableHintText={t`Mixed`}
            margin="none"
            stopPropagationOnClick
            onChange={event =>
              props.onChange(
                getVariableTypeToString()[event.target.value],
                props.nodeId
              )
            }
            inputStyle={{
              fontSize: 14,
              color: props.isHighlighted
                ? gdevelopTheme.listItem.selectedTextColor
                : undefined,
            }}
            id={props.id}
            disabled={props.disabled}
          >
            {getOptions()}
          </SelectField>
        </>
      )}
    </Line>
  );
});

export default VariableTypeSelector;
