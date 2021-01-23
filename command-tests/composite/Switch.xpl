<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :switch, :case and :default commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/switch-case">
    <Input>
      <xpl:switch key="2">
        <xpl:case key="1">
          <Wrong/>
        </xpl:case>
        <xpl:case key="2">
          <Right/>
        </xpl:case>
        <xpl:default>
          <Wrong/>
        </xpl:default>
      </xpl:switch>
    </Input>
    <Expected>
      <Right/>
    </Expected>
  </MustSucceed>
   
  <MustSucceed name="pass/no-break">
    <Input>
      <xpl:switch key="'b'">
        <xpl:case key="1">
          <Wrong/>
        </xpl:case>
        <xpl:default break="false">
          <Right/>
        </xpl:default>
        <xpl:case key="'b'" break="false">
          <Right2/>
        </xpl:case>
        <xpl:case key="'b'">
          <Right3/>
        </xpl:case>
        <xpl:case key="'b'">
          <Wrong/>
        </xpl:case>        
        <xpl:default>
          <Wrong/>
        </xpl:default>
      </xpl:switch>
    </Input>
    <Expected>
      <Right/>
      <Right2/>
      <Right3/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/case-identity">
    <Input>
      <A attr="a"/>
      <A attr="a"/>
      <A attr="b"/>
      <xpl:switch key="parent::*/A[1]">
        <xpl:case key="parent::*/parent::*/A[2]" break="false">
          <Equality/>
        </xpl:case>
        <xpl:case key="parent::*/parent::*/A[@attr='b']">
          <Wrong/>
        </xpl:case>
        <xpl:case key="parent::*/parent::*/A[2]" comparison="identity">
          <Wrong/>
        </xpl:case>
        <xpl:case key="parent::*/parent::*/A[1]" comparison="identity">
          <Identity/>
        </xpl:case>
      </xpl:switch>
    </Input>
    <Expected>
      <A attr="a"/>
      <A attr="a"/>
      <A attr="b"/>
      <Equality/>
      <Identity/>
    </Expected>
  </MustSucceed>  

  <MustSucceed name="pass/switch-repeat">
    <Input>
      <xpl:define name="A">
        <B/>
      </xpl:define>
      <xpl:switch key="true" repeat="true">
        <xpl:case key="true">
          <xpl:no-expand>
            <A/>
          </xpl:no-expand>
        </xpl:case>
      </xpl:switch>
    </Input>
    <Expected>
      <B/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/case-repeat">
    <Input>
      <xpl:define name="A">
        <B/>
      </xpl:define>
      <xpl:switch key="true()">
        <xpl:case key="true()" repeat="true" break="false">
          <xpl:no-expand>
            <A/>
          </xpl:no-expand>
        </xpl:case>
        <xpl:case key="true()">
          <xpl:no-expand>
            <A/>
          </xpl:no-expand>
        </xpl:case>        
      </xpl:switch>     
    </Input>
    <Expected>
      <B/>
      <A/>
    </Expected>  
  </MustSucceed>
  
  <MustSucceed name="pass/default-repeat">
    <Input>
      <xpl:define name="A">
        <B/>
      </xpl:define>
      <xpl:switch key="/*">
        <xpl:default repeat="true">
          <xpl:no-expand>
            <A/>
          </xpl:no-expand>
        </xpl:default>
      </xpl:switch>
    </Input>
    <Expected>
      <B/>
    </Expected>  
  </MustSucceed>
  
  <MustFail name="fail/missing-switch-key">
    <Input>
      <xpl:switch/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/missing-case-key">
    <Input>
      <xpl:switch key="1">
        <xpl:case/>
      </xpl:switch>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-switch-key">
    <Input>
      <xpl:switch key="))Z"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-case-key">
    <Input>
      <xpl:switch key="1">
        <xpl:case key="))Z"/>
      </xpl:switch>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-switch-repeat">
    <Input>
      <xpl:switch key="1" repeat="maybe"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-case-repeat">
    <Input>
      <xpl:switch key="1">
        <xpl:case key="1" repeat="not-sure"/>
      </xpl:switch>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-default-repeat">
    <Input>
      <xpl:switch key="1">
        <xpl:default repeat="twice"/>
      </xpl:switch>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-case-break">
    <Input>
      <xpl:switch key="1">
        <xpl:case key="1" break="eslaf"/>
      </xpl:switch>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-default-break">
    <Input>
      <xpl:switch key="1">
        <xpl:default break="eurt"/>
      </xpl:switch>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-case-comparison">
    <Input>
      <xpl:switch key="1">
        <xpl:case key="1" comparison="fuzzy"/>
      </xpl:switch>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bare-case">
    <Input>
      <xpl:case key="1"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bare-default">
    <Input>
      <xpl:default/>
    </Input>
  </MustFail>
 
  <Summary/>
</Root>