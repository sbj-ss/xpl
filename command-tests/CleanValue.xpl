<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :clean-value command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <xpl:define name="Passing">
    <xpl:attribute destination="following-sibling::*/xpl:clean-value" name="behavior">
      <xpl:content select="@behavior"/>
    </xpl:attribute>
    <Any>
      <xpl:clean-value expect="any">../123qqz!!@</xpl:clean-value>
    </Any>
    <Number>
      <xpl:clean-value expect="number" removeonnonmatch="true">-123.15</xpl:clean-value>
    </Number>
    <Hex>
      <xpl:clean-value expect="hex" removeonnonmatch="true">0x1234ABCD</xpl:clean-value>
    </Hex>
    <String>
      <xpl:clean-value expect="string" removeonnonmatch="true">test</xpl:clean-value>
    </String>
    <Path>
      <xpl:clean-value expect="path" removeonnonmatch="true">admin.xpl</xpl:clean-value>
    </Path>
  </xpl:define>

  <xpl:define name="Failing">
    <xpl:attribute destination="following-sibling::*/xpl:clean-value" name="behavior">
      <xpl:content select="@behavior"/>
    </xpl:attribute>
    <Number>
      <xpl:clean-value expect="number">&gt;qaz-123wsx</xpl:clean-value>
    </Number>
    <Hex>
      <xpl:clean-value expect="hex">0x1234ABQQCD</xpl:clean-value>
    </Hex>
    <String>
      <xpl:clean-value expect="string">'test'test</xpl:clean-value>
    </String>
    <Path>
      <xpl:clean-value expect="path">/../../admin.php</xpl:clean-value>
    </Path>
  </xpl:define>

  <DefaultBehavior>
    <Pass>
      <Number>
        <xpl:clean-value expect="number">1.2</xpl:clean-value>
      </Number>
    </Pass>
    <Fail>
      <Number>
        <xpl:clean-value expect="number">1.2.3</xpl:clean-value>
      </Number>
    </Fail>
  </DefaultBehavior>

  <Complain>
    <Pass>
      <Passing behavior="complain"/>
    </Pass>
    <Fail>
      <Failing behavior="complain"/>
    </Fail>
  </Complain>

  <Clear>
    <Pass>
      <Passing behavior="clear"/>
    </Pass>
    <Fail>
      <Failing behavior="clear"/>
    </Fail>
  </Clear>

  <Extract>
    <Pass>
      <Passing behavior="extract"/>
    </Pass>
    <Fail>
      <Failing behavior="extract"/>
    </Fail>
  </Extract>
</Root>
