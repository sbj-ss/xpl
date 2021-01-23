<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :command-supported command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/single">
    <Input>
      <xpl:command-supported name="command-supported"/>
    </Input>
    <Expected>true</Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/list">
    <Input>
      <xpl:include select="command[.='command-supported']">
        <xpl:command-supported/>
      </xpl:include>
    </Input>
    <Expected>
      <command>command-supported</command>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/custom-tag-name">
    <Input>
      <xpl:include select="Cmd[.='command-supported']">
        <xpl:command-supported tagname="Cmd"/>
      </xpl:include>
    </Input>
    <Expected>
      <Cmd>command-supported</Cmd>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/show-tags">
    <Input>
      <xpl:include select="command-supported">
        <xpl:command-supported showtags="true"/>
      </xpl:include>
    </Input>
    <Expected>
      <command-supported/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/no-repeat">
    <Input>
      <xpl:include select="./Wrong">
        <xpl:define name="command">
          <Wrong/>
        </xpl:define>
        <xpl:command-supported repeat="false"/>
      </xpl:include>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustFail name="fail/bad-tag-name">
    <Input>
      <xpl:command-supported tagname="@Z"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-show-tags">
    <Input>
      <xpl:command-supported showtags="partially"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/tag-name-with-show-tags">
    <Input>
      <xpl:command-supported tagname="Cmd" showtags="true"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/name-with-tag-name">
    <Input>
      <xpl:command-supported name="any" tagname="Cmd"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/name-with-show-tags">
    <Input>
      <xpl:command-supported name="any" showtags="true"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>