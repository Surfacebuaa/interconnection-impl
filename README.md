# 隐私计算平台互联互通协议参考实现

本仓库存放隐私计算平台互联互通协议参考实现的代码

## 构建

interconnection-impl引用了spu仓库代码，需要根据[spu构建前提](https://github.com/secretflow/spu/blob/main/CONTRIBUTING.md#build)在编译环境上安装好依赖库

然后执行以下构建指令：

```shell
bazel build ic_impl/ic_main
```

## 运行

### ECDH-PSI

本地同时执行以下两条指令：

```shell
bazel run ic_impl/ic_main -- -rank=0 -algo=ECDH_PSI -protocol_families=ECC \
        -in_path ic_impl/data/psi_1.csv -field_names id -out_path /tmp/p1.out \
        -parties=127.0.0.1:9530,127.0.0.1:9531
```

```shell
bazel run ic_impl/ic_main -- -rank=1 -algo=ECDH_PSI -protocol_families=ECC \
        -in_path ic_impl/data/psi_2.csv -field_names id -out_path /tmp/p2.out \
        -parties=127.0.0.1:9530,127.0.0.1:9531
```

### SS-LR

运行SS-LR之前，需要先启动Beaver服务。Beaver服务的代码位于SPU仓库中，需要将SPU代码克隆到本地，然后编译并启动Beaver服务：

```shell
git clone git@github.com:secretflow/spu.git
cd spu && bazel run libspu/mpc/semi2k/beaver/ttp_server:beaver_server_main -- -port=9449
```

启动Beaver服务后，本地同时执行以下两条指令：

```shell
bazel run ic_impl/ic_main -- -rank=0 -algo=SS_LR -protocol_families=SS \
        -dataset=ic_impl/data/perfect_logit_a.csv -has_label=true \
        -use_ttp=true -ttp_server_host=127.0.0.1:9449 \
        -parties=127.0.0.1:9530,127.0.0.1:9531
```

```shell
bazel run ic_impl/ic_main -- -rank=1 -algo=SS_LR -protocol_families=SS \
        -dataset=ic_impl/data/perfect_logit_b.csv -has_label=false \
        -use_ttp=true -ttp_server_host=127.0.0.1:9449 \
        -parties=127.0.0.1:9530,127.0.0.1:9531
```

## 环境变量传参

为满足北京金融科技产业联盟的调度层互联互通标准对算法组件接口的要求，interconnection-impl支持SS-LR算法从环境变量读取配置参数

当某个参数在环境变量和命令行选项都被指定时，优先选择读取环境变量参数

### 环境变量定义

| 环境变量                                               |                        参考值                        |                            描述                             |
|:---------------------------------------------------|:-------------------------------------------------:|:---------------------------------------------------------:|
| runtime.component.parameter.algo                   |                       ss_lr                       |                         algorithm                         |
| runtime.component.parameter.protocol_families      |                        ss                         |         comma-separated list of protocol families         |
| runtime.component.parameter.batch_size             |                        21                         |                    size of each batch                     |
| runtime.component.parameter.last_batch_policy      |                      discard                      |  policy to process the partial last batch of each epoch   |
| runtime.component.parameter.num_epoch              |                         1                         |                      number of epoch                      |
| runtime.component.parameter.l0_norm                |                         0                         |                      l0 penalty term                      |
| runtime.component.parameter.l1_norm                |                         0                         |                      l1 penalty term                      |
| runtime.component.parameter.l2_norm                |                        0.5                        |                      l2 penalty term                      |
| runtime.component.parameter.optimizer              |                        sgd                        |        optimization algorithm to speed up training        |
| runtime.component.parameter.learning_rate          |                      0.0001                       |         learning rate parameter in sgd optimizer          |
| runtime.component.parameter.sigmoid_mode           |                     minimax_1                     |               sigmoid approximation method                |
| runtime.component.parameter.protocol               |                      semi2k                       |                     ss protocol type                      |
| runtime.component.parameter.field                  |                        64                         |        field type, 64 for Ring64, 128 for Ring128         |
| runtime.component.parameter.fxp_bits               |                        18                         |       number of fraction bits of fixed-point number       |
| runtime.component.parameter.trunc_mode             |                   probabilistic                   |                      truncation mode                      |
| runtime.component.parameter.shard_serialize_format |                        raw                        | serialization format used for communicating secret shares |
| runtime.component.parameter.use_ttp                |                       true                        |               whether to use beaver service               |
| runtime.component.parameter.ttp_server_host        |                      ip:port                      |   remote ip:port or load-balance uri of beaver service    |
| runtime.component.parameter.ttp_session_id         |               interconnection-root                |               session id of beaver service                |
| runtime.component.parameter.ttp_adjust_rank        |                         0                         |      which rank do adjust rpc call to beaver service      |
| system.storage                                     |                file://path/to/root                |             root path of input / output file              |
| runtime.component.input.train_data                 | {"namespace":"data","name":"perfect_logit_a.csv"} |           relative path and name of input file            |
| runtime.component.parameter.skip_rows=1            |                         1                         |            number of skipped rows from dataset            |
| runtime.component.parameter.has_label              |                       true                        |       if true, label is the last column of dataset        |
| runtime.component.output.train_data                |     {"namespace":"output","name":"result_a"}      |           relative path and name of output file           |

## FAQ

若构建失败并提示 `Host key verification failed`，解决方式如下:

* 首先，[向github账户添加SSH key](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account)

* 然后在本地执行以下指令(以选择 RSA 作为 SSH key 类型为例)：

```shell
ssh-keyscan -t rsa github.com >> ~/.ssh/known_hosts
```