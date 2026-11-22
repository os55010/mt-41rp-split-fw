# mt-41rp split ファームウェア（ZMK）

[mt-41rp split](https://github.com/os55010-org/mt-41rp-split)（左右分割・無線・41キー＋トラックボール）用の ZMK 設定。
[mt42](https://github.com/os55010/mt42) / [ortho42tb](https://github.com/os55010/ortho42tb) をもとにしている。

## 構成
- MCU: Seeed XIAO nRF52840 × 2（BLE 分割）
- **右手（トラックボール側）がセントラル**。ホストとの接続、ZMK Studio、トラックボールの処理は右手で行う
- トラックボール: PAW3222（[zmk-driver-paw3222](https://github.com/os55010/zmk-driver-paw3222)）

## ビルド
GitHub Actions（`.github/workflows/build.yml`）で次の 3 つがビルドされる。

| ファイル | 書き込み先 |
|---|---|
| `mt41rp_split_right-xiao_ble_zmk-zmk.uf2`（Studio 有効） | 右手 |
| `mt41rp_split_left-xiao_ble_zmk-zmk.uf2` | 左手 |
| `settings_reset-xiao_ble_zmk-zmk.uf2` | ペアリングをやり直すとき左右両方に |

## キーの位置番号
```
  0  1  2  3  4          5  6  7  8  9
 10 11 12 13 14         15 16 17 18 19
 20 21 22 23 24         25 26 27 28 29
 30 31 32 33 34 35   36 37 (TB) 38 39 40
```
35 は左手の MCU 列の親指キー、36 は右手の MCU 列の親指キー、TB はトラックボール。

## ピンアサイン（ハードウェアの README と基板のネットから確認）
| ネット | 左手 | 右手 |
|---|---|---|
| row1〜row4 | D0〜D3（P0.02 / P0.03 / P0.28 / P0.29） | 同左 |
| col1〜col5 | D6〜D10（P1.11〜P1.15） | 同左 |
| col6 | D4（P0.04、親指キー） | ― |
| sdio / sclk | ― | D4（P0.04）/ D5（P0.05） |
| cs / mot | ― | NFC1（P0.09）/ NFC2（P0.10） |

- マトリクスは COL2ROW。右手は `col-offset = <6>` で、全体を 4 行 × 11 列として扱う
- 右手は NFC ピンを GPIO に使うので、overlay に `&uicr { nfct-pins-as-gpios; };` を入れている
- D6 / D7 を col に使うので `xiao_serial` は無効化している

## 状態 LED（XIAO の RGB LED、`src/status_leds.c`）
| 状態 | LED |
|---|---|
| バッテリーに充電中 | 緑で点灯 |
| ペアリング中 | 青で点滅（0.5 秒ごと） |

- 充電中かどうかは、XIAO の充電 IC（BQ25101）の CHG 出力（P0.17）で判定する。USB をつないでいても、バッテリーがない・満充電なら消える
- ペアリング中の判定
  - 右手: 今の BT プロファイルにホストが登録されていない（`&bt BT_CLR` 後など）
  - 左手: まだ右手とボンディングされておらず、つながっていない（settings_reset 後など）
- 色やピンを変える: `mt41rp_split.dtsi` の `status_leds` ノード（led0 = 赤, led1 = 緑, led2 = 青）
- 無効にする: conf に `CONFIG_MT41RP_STATUS_LEDS=n`
