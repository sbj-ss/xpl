<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :delete command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/delete-element">
    <Input>
      <A/>
      <xpl:delete select="preceding-sibling::A"/>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustSucceed name="pass/delete-attr">
    <Input>
      <A attr="1"/>
      <xpl:delete select="preceding-sibling::A/@attr"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/delete-text">
    <Input>
      <A>text</A>
      <xpl:delete select="preceding-sibling::A/text()"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/delete-cdata">
    <Input>
      <A>
        <![CDATA[a<b]]>
      </A>
      <xpl:delete select="preceding-sibling::A/text()"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/delete-parent">
    <Input>
      <A>
        <xpl:delete select="parent::A"/>
      </A>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustSucceed name="pass/delete-ancestor">
    <Input>
      <A>
        <B>
          <xpl:delete select="ancestor::A"/>
        </B>
      </A>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustSucceed name="pass/delete-nested">
    <Input>
      <A>
        <A/>
      </A>
      <xpl:delete select="preceding-sibling::A/ancestor-or-self::A"/>
    </Input>
    <Expected>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/no-select">
    <Input>
      <xpl:delete/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-select">
    <Input>
      <xpl:delete select="))Z"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>