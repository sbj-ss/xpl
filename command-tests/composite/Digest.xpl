<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :digest command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/valid-methods">
    <Input>
      <md4>
        <xpl:digest method="md4">А ну-ка, песню нам пропой, весёлый ветер,</xpl:digest>
      </md4>
      <md5>
        <xpl:digest method="md5">Весёлый ветер, весёлый ветер!</xpl:digest>
      </md5>
      <ripemd-160>
        <xpl:digest method="ripemd-160">Моря и горы ты обшарил все на свете</xpl:digest>
      </ripemd-160>
      <sha1>
        <xpl:digest method="sha1">И все на свете песенки слыхал.</xpl:digest>
      </sha1>
      <sha-224>
        <xpl:digest method="sha-224">Спой нам, ветер, про славу и смелость,</xpl:digest>
      </sha-224>
      <sha-256>
        <xpl:digest method="sha-256">Про учёных, героев, бойцов,</xpl:digest>
      </sha-256>
      <sha-384>
        <xpl:digest method="sha-384">Чтоб сердце загорелось,</xpl:digest>
      </sha-384>
      <sha-512>
        <xpl:digest method="sha-512">Чтоб каждому хотелось</xpl:digest>
      </sha-512>
      <whirlpool>
        <xpl:digest method="whirlpool">Догнать и перегнать отцов!</xpl:digest>
      </whirlpool>
    </Input>
    <Expected>
      <md4>B57BC99E8ED7B38CE2E286ACC989C4AD</md4>
      <md5>A07866E912FF9D68045CC735CB52DF79</md5>
      <ripemd-160>7BF21407757E26F1FA963DD8753B5302712C15E3</ripemd-160>
      <sha1>435DD5D28064E2776043E256F7FFCDD4D836FB8C</sha1>
      <sha-224>8EA6154BF7A695675D263E711C26C257751CF562D4CBAFB8B902B64F</sha-224>
      <sha-256>BC4CA02D196421A71428B2BB252D51EC25885727C9C7B7CB915576B7AAF24971</sha-256>
      <sha-384>2B2E0A0AE0D8A9D07DFBC5F8C6C92C57E3DF92F79E88C1024B6FF8DCB2686A51185E542A6A2787D13EEB1821DCB25746</sha-384>
      <sha-512>257AD667CA6CAB6FFD7E53EEDBD8E4ED38BE789251ACB41F6A4CB456BB736460AB33B4A85D0D02310AE2370DA48A821E07D862C814E686B2FC450E9BEB4383B6</sha-512>
      <whirlpool>AF886AAB1B88708F64F9C25FBBA571C662B056107BA537E58D0846B89887E7BB18092740C3B88A9E2E615D79859E142DB92E031462934647B49309E9537F1791</whirlpool>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/no-digest">
    <Input>
      <xpl:digest/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-digest">
    <Input>
      <xpl:digest method="random"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>