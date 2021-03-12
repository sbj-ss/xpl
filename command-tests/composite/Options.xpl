<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :get-option and :set-option commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <xpl:set-sa-mode password="1111111"/>
  <xpl:set-option name="EnableAssertions">true</xpl:set-option>
  <xpl:set-sa-mode enable="false"/>
  
  <MustSucceed name="pass/get-single">
    <Input>
      <xpl:get-option name="EnableAssertions"/>
    </Input>
    <Expected>true</Expected>
  </MustSucceed>

  <MustSucceed name="pass/get-all">
    <Input>
      <xpl:include select="./option[@name='EnableAssertions']">
        <xpl:get-option/>
      </xpl:include>
    </Input>
    <Expected>
      <option name="EnableAssertions" type="bool">true</option>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/get-all-custom-tag-name">
    <Input>
      <Outer>
        <xpl:include select="./*[local-name()='Opt'][@name='EnableAssertions']">
          <xpl:get-option xmlns:ns-b="http://b.com" tagname="ns-b:Opt"/>
        </xpl:include>
      </Outer>
    </Input>
    <Expected>
      <Outer xmlns:ns-b="http://b.com">
        <ns-b:Opt name="EnableAssertions" type="bool">true</ns-b:Opt>
      </Outer>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/get-all-show-tags">
    <Input>
      <xpl:include select="./EnableAssertions">
        <xpl:get-option showtags="true"/>
      </xpl:include>
    </Input>
    <Expected>
      <EnableAssertions type="bool">true</EnableAssertions>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/get-all-no-repeat">
    <Input>
      <xpl:define name="option">
        <Processed/>
      </xpl:define>
      <xpl:include select="./Processed">
        <xpl:get-option repeat="false"/>
      </xpl:include>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/set-get">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:set-option name="ProxyPort">1234</xpl:set-option>
      <xpl:get-option name="ProxyPort"/>
      <xpl:set-option name="ProxyPort" todefault="true"/>
    </Input>
    <Expected>1234</Expected>
  </MustSucceed>

  <MustSucceed name="pass/show-passwords">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:set-option name="ProxyPassword">123</xpl:set-option>
      <Hidden>
        <xpl:get-option name="ProxyPassword"/>
      </Hidden>
      <Visible>
        <xpl:get-option name="ProxyPassword" showpasswords="true"/>
      </Visible>
      <xpl:set-option name="ProxyPassword" todefault="true"/>
    </Input>
    <Expected>
      <Hidden>[hidden]</Hidden>
      <Visible>123</Visible>
    </Expected>
  </MustSucceed>
 
  <MustFail name="fail/get-bad-tag-name">
    <Input>
      <xpl:get-option tagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-bad-show-tags">
    <Input>
      <xpl:get-option showtags="only-short"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-bad-repeat">
    <Input>
      <xpl:get-option repeat="maybe"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-bad-show-passwords">
    <Input>
      <xpl:get-option showpasswords="when nobody's watching"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-unknown">
    <Input>
      <xpl:get-option name="HuntHeffalumps"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-access-denied">
    <Input>
      <xpl:get-option showpasswords="true"/>
    </Input>
  </MustFail>

  <MustFail name="fail/set-no-name">
    <Input>
      <xpl:set-option/>
    </Input>
  </MustFail>

  <MustFail name="fail/set-access-denied">
    <Input>
      <xpl:set-option name="LuciferCompatibility">true</xpl:set-option>
    </Input>
  </MustFail>

  <MustFail name="fail/set-unknown">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:set-option name="HuntHeffalumps">whatever</xpl:set-option>
    </Input>
  </MustFail>

  <MustFail name="fail/set-bad-value">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:set-option name="UseConsoleColors">bright</xpl:set-option>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>