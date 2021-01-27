<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :stack-clear, :stack-is-empty, :stack-localize, :stack-pop and :stack-push commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/simple">
    <Input>
      <InitialStackState>
        <xpl:stack-is-empty/>
      </InitialStackState>    
      <Outer>
        <Wrapper xmlns:ns-b="http://b.com">
          <xpl:stack-push>
            <ns-b:A/>
          </xpl:stack-push>
        </Wrapper>
        <IntermediateStackState>
          <xpl:stack-is-empty/>
        </IntermediateStackState>        
        <xpl:stack-pop/>
      </Outer>
      <FinalStackState>
        <xpl:stack-is-empty/>
      </FinalStackState>
    </Input>
    <Expected>
      <InitialStackState>true</InitialStackState>
      <Outer xmlns:ns-b="http://b.com">
        <Wrapper xmlns:ns-b="http://b.com"/>
        <IntermediateStackState>false</IntermediateStackState>
        <ns-b:A/>
      </Outer>
      <FinalStackState>true</FinalStackState>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/localize">
    <Input>
      <xpl:stack-push>
        <A/>
      </xpl:stack-push>
      <Local>
        <xpl:stack-localize>
          <xpl:stack-pop/>
        </xpl:stack-localize>
      </Local>
      <xpl:stack-pop/>
    </Input>
    <Expected>
      <Local/>
      <A/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/localize-repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:stack-localize repeat="true">
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:stack-localize>
    </Input>
    <Expected>
      <ProcessedA/>
    </Expected>
  </MustSucceed>
  
  <MustSucced name="pass/pop-repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:stack-push>
        <xpl:no-expand>
          <A/>
          <A/>
        </xpl:no-expand>
      </xpl:stack-push>
      <Repeat>
        <xpl:stack-pop/>
      </Repeat>
      <NoRepeat>
        <xpl:stack-pop repeat="false"/>
      </NoRepeat>
      <Expected>
        <Repeat>
          <ProcessedA/>
        </Repeat>
        <NoRepeat>
          <A/>
        </NoRepeat>
      </Expected>
    </Input>
  </MustSucced>
  
  <MustFail name="fail/localize-bad-repeat">
    <Input>
      <xpl:stack-localize repeat="not now"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/pop-bad-repeat">
    <Input>
      <xpl:stack-pop repeat="sure"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>