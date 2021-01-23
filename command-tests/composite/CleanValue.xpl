<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :clean-value command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/extract-valid">
    <Input>
      <Any>
        <xpl:clean-value expect="any" behavior="extract">../123qqz!!@</xpl:clean-value>
      </Any>
      <Number>
        <xpl:clean-value expect="number" behavior="extract">-123.15</xpl:clean-value>
      </Number>
      <Hex>
        <xpl:clean-value expect="hex" behavior="extract">0x1234ABCD</xpl:clean-value>
      </Hex>
      <String>
        <xpl:clean-value expect="string" behavior="extract">test</xpl:clean-value>
      </String>
      <Path>
        <xpl:clean-value expect="path" behavior="extract">admin.xpl</xpl:clean-value>
      </Path>
    </Input>
    <Expected>
      <Any>../123qqz!!@</Any>
      <Number>-123.15</Number>
      <Hex>0x1234ABCD</Hex>
      <String>test</String>
      <Path>admin.xpl</Path>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/extract-invalid">
    <Input>
      <Number>
        <xpl:clean-value expect="number" behavior="extract">-123.15ABC</xpl:clean-value>
      </Number>
      <Hex>
        <xpl:clean-value expect="hex" behavior="extract">0x1234ABCDфпень</xpl:clean-value>
      </Hex>
      <String>
        <xpl:clean-value expect="string" behavior="extract">'test'</xpl:clean-value>
      </String>
      <Path>
        <xpl:clean-value expect="path" behavior="extract">../admin.xpl</xpl:clean-value>
      </Path>
    </Input>
    <Expected>
      <Number>-123.15</Number>
      <Hex>0x1234ABCD</Hex>
      <String>`test`</String>
      <Path>./admin.xpl</Path>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/clear-valid">
    <Input>
      <Any>
        <xpl:clean-value expect="any" behavior="clear">../123qqz!!@</xpl:clean-value>
      </Any>
      <Number>
        <xpl:clean-value expect="number" behavior="clear">-123.15</xpl:clean-value>
      </Number>
      <Hex>
        <xpl:clean-value expect="hex" behavior="clear">0x1234ABCD</xpl:clean-value>
      </Hex>
      <String>
        <xpl:clean-value expect="string" behavior="clear">test</xpl:clean-value>
      </String>
      <Path>
        <xpl:clean-value expect="path" behavior="clear">admin.xpl</xpl:clean-value>
      </Path>
    </Input>
    <Expected>
      <Any>../123qqz!!@</Any>
      <Number>-123.15</Number>
      <Hex>0x1234ABCD</Hex>
      <String>test</String>
      <Path>admin.xpl</Path>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/clear-invalid">
    <Input>
      <Number>
        <xpl:clean-value expect="number" behavior="clear">-123.15ABC</xpl:clean-value>
      </Number>
      <Hex>
        <xpl:clean-value expect="hex" behavior="clear">0x1234ABCDфпень</xpl:clean-value>
      </Hex>
      <String>
        <xpl:clean-value expect="string" behavior="clear">'test'</xpl:clean-value>
      </String>
      <Path>
        <xpl:clean-value expect="path" behavior="clear">../admin.xpl</xpl:clean-value>
      </Path>
    </Input>
    <Expected>
      <Number/>
      <Hex/>
      <String/>
      <Path/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/complain-valid">
    <Input>
      <Any>
        <xpl:clean-value expect="any" behavior="complain">../123qqz!!@</xpl:clean-value>
      </Any>
      <Number>
        <xpl:clean-value expect="number" behavior="complain">-123.15</xpl:clean-value>
      </Number>
      <Hex>
        <xpl:clean-value expect="hex" behavior="complain">0x1234ABCD</xpl:clean-value>
      </Hex>
      <String>
        <xpl:clean-value expect="string" behavior="complain">test</xpl:clean-value>
      </String>
      <Path>
        <xpl:clean-value expect="path" behavior="complain">admin.xpl</xpl:clean-value>
      </Path>
    </Input>
    <Expected>
      <Any>../123qqz!!@</Any>
      <Number>-123.15</Number>
      <Hex>0x1234ABCD</Hex>
      <String>test</String>
      <Path>admin.xpl</Path>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/complain-invalid-number">
    <Input>
      <xpl:clean-value expect="number" behavior="complain">-123.15ABC</xpl:clean-value>
    </Input>
  </MustFail>
  
  <MustFail name="fail/complain-invalid-hex">
    <Input>
      <xpl:clean-value expect="hex" behavior="complain">0x1234ABCDфпень</xpl:clean-value>
    </Input>
  </MustFail>
  
  <MustFail name="fail/complain-invalid-string">
    <Input>
      <xpl:clean-value expect="string" behavior="complain">'test'</xpl:clean-value>
    </Input>
  </MustFail>
  
  <MustFail name="fail/complain-invalid-path">
    <Input>
      <xpl:clean-value expect="path" behavior="complain">../admin.xpl</xpl:clean-value>
    </Input>
  </MustFail>
  
  <MustFail name="fail/default-behavior-invalid-number">
    <Input>
      <xpl:clean-value expect="number">jabberwocky</xpl:clean-value>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-expect">
    <Input>
      <xpl:clean-value expect="animal"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-behavior">
    <Input>
      <xpl:clean-value behavior="undefined"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>
