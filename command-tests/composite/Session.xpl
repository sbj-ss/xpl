<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :session-* and :get-thread-id commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/shared">
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
      <AAfterSetObject>
        <ns-b:A xmlns:ns-b="http://b.com">
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

  <MustSucceed name="pass/local">
    <Input>
      <AllBefore>
        <xpl:session-get-object local="true"/>
      </AllBefore>

      <xpl:session-set-object name="a" xmlns:ns-b="http://b.com" local="true">
        <ns-b:A>
          <Content>text</Content>
        </ns-b:A>
      </xpl:session-set-object>     
      <xpl:session-set-object name="b" local="true">
        <B/>
      </xpl:session-set-object>
 
      <AllAfterSetObject>
        <xpl:session-get-object local="true"/>
      </AllAfterSetObject>
      <AAfterSetObject>
        <xpl:session-get-object name="a" local="true"/>
      </AAfterSetObject>
      <AAfterGetObjectWithSelect>
        <xpl:session-get-object name="a" select="ns-b:A/node()" local="true"/>
      </AAfterGetObjectWithSelect>
      <ContainsAAfterSetObject>
        <xpl:session-contains-object name="a" local="true"/>
      </ContainsAAfterSetObject>
      <ContainsBAfterSetObject>
        <xpl:session-contains-object name="b" local="true"/>
      </ContainsBAfterSetObject>
 
      <xpl:session-remove-object name="b" local="true"/>
     
      <BAfterRemoveObject>
        <xpl:session-get-object name="b" local="true"/>
      </BAfterRemoveObject>
      <ContainsAAfterRemoveObject>
        <xpl:session-contains-object name="a" local="true"/>
      </ContainsAAfterRemoveObject>
      <ContainsBAfterRemoveObject>
        <xpl:session-contains-object name="b" local="true"/>
      </ContainsBAfterRemoveObject>

      <xpl:session-clear local="true"/>
  
      <AllAfterClear>
        <xpl:session-get-object local="true"/>
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
      <AAfterSetObject>
        <ns-b:A xmlns:ns-b="http://b.com">
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
  
  <MustSucceed name="pass/shared-vs-local">
    <Input>
      <xpl:session-set-object name="a" local="true">
        <A/>
      </xpl:session-set-object>
      <Global>
        <xpl:session-get-object name="a"/>
      </Global>
      <Local>
        <xpl:session-get-object name="a" local="true"/>
      </Local>
      
      <xpl:session-remove-object name="a"/>
      <AfterGlobalRemoval>
        <xpl:session-contains-object name="a" local="true"/>
      </AfterGlobalRemoval>
      
      <xpl:session-remove-object name="a" local="true"/>
      <AfterLocalRemoval>
        <xpl:session-contains-object name="a" local="true"/>
      </AfterLocalRemoval>
    </Input>
    <Expected>
      <Global/>
      <Local>
        <A/>
      </Local>
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

  <MustFail name="fail/set-object-bad-local">
    <Input>
      <xpl:session-set-object local="if supported"/>
    </Input>
  </MustFail>

  <MustFail name="fail/contains-object-bad-local">
    <Input>
      <xpl:session-contains-object local="maybe"/>
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

  <MustFail name="fail/get-object-bad-local">
    <Input>
      <xpl:session-get-object local="mostly"/>
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

  <MustFail name="fail/remove-object-bad-local">
    <Input>
      <xpl:session-remove-object local="partially"/>
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
  
  <MustFail name="fail/clear-bad-local">
    <Input>
      <xpl:session-clear local="not at all"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/get-id-bad-local">
    <Input>
      <xpl:session-get-id local="very"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>