<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :session-* and :get-thread-id commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <AllBefore>
        <xpl:session-get-object/>
      </AllBefore>
      
      <xpl:session-set-object name="a" xmlns:ns-b="http://b.com">
        <ns-b:A>
          <Content>text</Content>
        </ns-b:A>
      </xpl:session-set-object>
      <xpl:session-set-object name="b">
        <B/>
      </xpl:session-set-object>

      <AllAfterSetObject>
        <xpl:session-get-object/>
      </AllAfterSetObject>
      <AAfterSetObject>
        <xpl:session-get-object name="a"/>
      </AAfterSetObject>
      <AAfterGetObjectWithSelect>
        <xpl:session-get-object name="a" select="ns-b:A/node()"/>
      </AAfterGetObjectWithSelect>
      <ContainsAAfterSetObject>
        <xpl:session-contains-object name="a"/>
      </ContainsAAfterSetObject>
      <ContainsBAfterSetObject>
        <xpl:session-contains-object name="b"/>
      </ContainsBAfterSetObject>
      
      <xpl:session-remove-object name="b"/>
      
      <BAfterRemoveObject>
        <xpl:session-get-object name="b"/>
      </BAfterRemoveObject>
      <ContainsAAfterRemoveObject>
        <xpl:session-contains-object name="a"/>
      </ContainsAAfterRemoveObject>
      <ContainsBAfterRemoveObject>
        <xpl:session-contains-object name="b"/>
      </ContainsBAfterRemoveObject>
            
      <xpl:session-clear/>
      
      <AllAfterClear>
        <xpl:session-get-object/>
      </AllAfterClear>
    </Input>
    <Expected>
      <AllBefore/>
      <AllAfterSetObject xmlns:ns-b="http://b.com">
        <a>
          <ns-b:A>
            <Content>text</Content>
          </ns-b:A>
        </a>
        <b>
          <B/>
        </b>
      </AllAfterSetObject>
      <AAfterSetObject xmlns:ns-b="http://b.com">
        <ns-b:A>
          <Content>text</Content>
        </ns-b:A>
      </AAfterSetObject>
      <AAfterGetObjectWithSelect>
        <Content>text</Content>
      </AAfterGetObjectWithSelect>
      <ContainsAAfterSetObject>true</ContainsAAfterSetObject>
      <ContainsBAfterSetObject>true</ContainsBAfterSetObject>
      <BAfterRemoveObject/>
      <ContainsAAfterRemoveObject>true</ContainsAAfterRemoveObject>
      <ContainsBAfterRemoveObject>false</ContainsBAfterRemoveObject>
      <AllAfterClear/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/thread-local">
    <Input>
      <xpl:session-set-object name="a" threadlocal="true">
        <A/>
      </xpl:session-set-object>
      <Global>
        <xpl:session-get-object name="a"/>
      </Global>
      <Local>
        <xpl:session-get-object name="a" threadlocal="true"/>
      </Local>
      <LocalIndirect>
        <xpl:session-get-object>
          <xpl:attribute name="name">a<xpl:get-thread-id/></xpl:attribute>
        </xpl:session-get-object>
      </LocalIndirect>
      
      <xpl:session-remove-object name="a"/>
      <AfterGlobalRemoval>
        <xpl:session-contains-object name="a" threadlocal="true"/>
      </AfterGlobalRemoval>
      
      <xpl:session-remove-object name="a" threadlocal="true"/>
      <AfterLocalRemoval>
        <xpl:session-contains-object name="a" threadlocal="true"/>
      </AfterLocalRemoval>
    </Input>
    <Expected>
      <Global/>
      <Local>
        <A/>
      </Local>
      <LocalIndirect>
        <A/>
      </LocalIndirect>
      <AfterGlobalRemoval>true</AfterGlobalRemoval>
      <AfterLocalRemoval>false</AfterLocalRemoval>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/no-repeat">
    <Input>
      <xpl:session-set-object name="a">
        <A/>
      </xpl:session-set-object>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <Repeat>
        <xpl:session-get-object name="a"/>
      </Repeat>
      <NoRepeat>
        <xpl:session-get-object name="a" repeat="false"/>
      </NoRepeat>
    </Input>
    <Expected>
      <Repeat>
        <ProcessedA/>
      </Repeat>
      <NoRepeat>
        <A/>
      </NoRepeat>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/set-object-missing-name">
    <Input>
      <xpl:session-set-object/>
    </Input>
  </MustFail>

  <MustFail name="fail/set-object-bad-name">
    <Input>
      <xpl:session-set-object name="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/set-object-bad-thread-local">
    <Input>
      <xpl:session-set-object threadlocal="if supported"/>
    </Input>
  </MustFail>

  <MustFail name="fail/contains-object-bad-thread-local">
    <Input>
      <xpl:session-contains-object threadlocal="maybe"/>
    </Input>
  </MustFail>

  <MustFail name="fail/contains-object-bad-name">
    <Input>
      <xpl:session-contains-object name="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-object-bad-repeat">
    <Input>
      <xpl:session-get-object repeat="as you wish"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-object-bad-thread-local">
    <Input>
      <xpl:session-get-object threadlocal="mostly"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/get-object-bad-select">
    <Input>
      <xpl:session-get-object select="((Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-object-bad-name">
    <Input>
      <xpl:session-get-object name="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-object-bad-thread-local">
    <Input>
      <xpl:session-remove-object threadlocal="partially"/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-object-missing-name">
    <Input>
      <xpl:session-remove-object/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-object-bad-name">
    <Input>
      <xpl:session-remove-object name="#"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>