/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import createWidget from '../widget/index'
import { getLocale } from '../../../../common/locale'

import {
  WidgetWrapper,
  Header,
  Content,
  WelcomeText,
  ActionsWrapper,
  CallButton,
  TogetherIcon,
  StyledTitle,
  Privacy,
  PrivacyLink
} from './style'
import { StyledTitleTab } from '../widgetTitleTab'
import BraveTogetherIcon from './assets/brave-together-icon'
import getRandomBase64 from '../../../getRandomBytes'

interface Props {
  showContent: boolean
  onShowContent: () => void
}

class Together extends React.PureComponent<Props, {}> {

  getButtonText = () => {
    return getLocale('togetherWidgetStartButton')
  }

  renderTitle () {
    return (
      <Header>
        <StyledTitle>
          <TogetherIcon>
            <BraveTogetherIcon />
          </TogetherIcon>
          <>
            {getLocale('togetherWidgetTitle')}
          </>
        </StyledTitle>
      </Header>
    )
  }

  renderTitleTab () {
    const { onShowContent } = this.props

    return (
      <StyledTitleTab onClick={onShowContent}>
        {this.renderTitle()}
      </StyledTitleTab>
    )
  }

  shouldCreateCall = (event: any) => {
    event.preventDefault()
    const roomId = getRandomBase64(32)
    window.open(`https://together.brave.com/${roomId}`, '_self', 'noopener')
  }

  render () {
    const {
      showContent
    } = this.props

    if (!showContent) {
      return this.renderTitleTab()
    }

    return (
      <WidgetWrapper>
          {this.renderTitle()}
          <Content>
            <WelcomeText>
              {getLocale('togetherWidgetWelcomeTitle')}
            </WelcomeText>
            <ActionsWrapper>
              <CallButton onClick={this.shouldCreateCall}>
                {getLocale('togetherWidgetStartButton')}
              </CallButton>
              <Privacy>
                <PrivacyLink
                  rel={'noopener'}
                  target={'_blank'}
                  href={'https://brave.com/privacy/#brave-together-learn'}
                >
                  {getLocale('togetherWidgetAboutData')}
                </PrivacyLink>
              </Privacy>
            </ActionsWrapper>
          </Content>
      </WidgetWrapper>
    )
  }
}

export const TogetherWidget = createWidget(Together)
