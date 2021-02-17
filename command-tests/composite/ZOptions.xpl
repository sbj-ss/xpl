<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :get-option command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/single">
    <Input>
      <xpl:get-option name="EnableAssertions"/>
    </Input>
    <Expected>true</Expected>
  </MustSucceed>

  <MustSucceed name="pass/all">
    <Input>
      <xpl:include select="./option[@name='EnableAssertions']">
        <xpl:get-option/>
      </xpl:include>
    </Input>
    <Expected>
      <option name="EnableAssertions" type="bool">true</option>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/custom-tag-name">
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

  <MustSucceed name="pass/show-tags">
    <Input>
      <xpl:include select="./EnableAssertions">
        <xpl:get-option showtags="true"/>
      </xpl:include>
    </Input>
    <Expected>
      <EnableAssertions type="bool">true</EnableAssertions>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/no-repeat">
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

<!-- To test this we need :set-option as default ProxyPassword is empty -->
<!-- 
  <MustSucceed name="pass/show-passwords">
    <Input>
      <Hidden>
        <xpl:get-option name=""/>
      </Hidden>
      <Visible>
        <xpl:get-option name="" showpasswords="true"/>
      </Visible>
    </Input>
    <Expected></Expected>
  </MustSucceed>
 -->
 
  <MustFail name="fail/bad-tag-name">
    <Input>
      <xpl:get-option tagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-show-tags">
    <Input>
      <xpl:get-option showtags="only-short"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:get-option repeat="maybe"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-show-passwords">
    <Input>
      <xpl:get-option showpasswords="when nobody's watching"/>
    </Input>
  </MustFail>

  <MustFail name="fail/unknown">
    <Input>
      <xpl:get-option name="HuntHeffalumps"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>