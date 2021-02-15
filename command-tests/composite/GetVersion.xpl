<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :get-version command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/major">
    <Input>
      <xpl:get-version part="major"/>
    </Input>
    <Expected>2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/minor">
    <Input>
      <xpl:get-version part="minor"/>
    </Input>
    <Expected>0</Expected>
  </MustSucceed>

  <MustSucceed name="pass/full">
    <Input>
      <xpl:regex-match regex="^C XPL">
        <xpl:text><xpl:get-version/></xpl:text>
      </xpl:regex-match>
    </Input>
    <Expected>true</Expected>
  </MustSucceed>

  <MustSucceed name="pass/libs">
    <Input>
      <xpl:include select="library[@name='libxml2']/@name">
        <xpl:get-version part="libs"/>
      </xpl:include>
    </Input>
    <Expected>libxml2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/libs-custom-tag-name">
    <Input>
      <xpl:include select="ns-a:lib[@name='libxml2']/@name">
        <xpl:get-version part="libs" tagname="ns-a:lib"/>
      </xpl:include>
    </Input>
    <Expected>libxml2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/libs-no-repeat">
    <Input>
      <xpl:define name="library"/>
      <xpl:include select="library[@name='libxml2']/@name">
        <xpl:get-version part="libs" repeat="false"/>
      </xpl:include>
    </Input>
    <Expected>libxml2</Expected>
  </MustSucceed>

  <MustSucceed name="pass/no-empty">
    <Input>
      <xpl:define name="library"/>
      <xpl:include select="library[@name=''] | library[@compiled=''] | library[@running='']">
        <xpl:get-version part="libs" repeat="false"/>
      </xpl:include>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustFail name="fail/bad-part">
    <Input>
      <xpl:get-version part="motherboard"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-tagname">
    <Input>
      <xpl:get-version tagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:get-version repeat="twice"/>
    </Input>
  </MustFail>
</Root>