<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :delete, :for-each and :with - very bad cases 
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/with-self">
    <Input>
      <xpl:with select="."/>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/with-ancestor">
    <Input>
      <Container>
        <xpl:with select="parent::*"/>
      </Container>
    </Input>
    <Expected>
      <Container/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/delete-linear-selection">
    <Input>
      <A/>
      <A/>
      <xpl:with select="preceding-sibling::A">
        <xpl:delete select="//A"/>
        <B/>
      </xpl:with>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/delete-hierarchy">
    <Input>
      <A>
        <A/>
      </A>
      <xpl:with select="//A">
        <xpl:delete select="//A"/>
        <B/>
      </xpl:with>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/delete-with">
    <Input>
      <xpl:with select="following-sibling::A">
        <B/>
        <xpl:delete select="//xpl:with"/>
      </xpl:with>
      <A/>
      <A/>
    </Input>
    <Expected>
      <B/>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/delete-with-by-content">
    <Input>
      <xpl:with select="following-sibling::*">
        <B/>
        <xpl:content/>
      </xpl:with>
      <DoNotTouch>
        <xpl:delete select="//xpl:with"/>
      </DoNotTouch>
      <A/>
    </Input>
    <Expected>
      <B/>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/modify-selection">
    <Input>
      <C/>
      <C/>
      <xpl:with select="preceding-sibling::C">
        <xpl:with select="parent::*/following-sibling::C">
          <Q/>
        </xpl:with>
      </xpl:with>
    </Input>
    <Expected>
      <Q/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/with-fatal">
    <Input>
      <A/>
      <A/>
      <xpl:with select="preceding-sibling::A">
        <xpl:fatal>
          <Root>a nice place for :fatal indeed</Root>
        </xpl:fatal>
      </xpl:with>
    </Input>
    <Expected>a nice place for :fatal indeed</Expected>
  </MustSucceed>
  <Summary/>
</Root>