import 'package:dio/dio.dart';
import 'package:flutter/material.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

import '../widgets/gradient_background.dart';
import '../widgets/light_chart.dart';

class CarRCScreen extends StatelessWidget {
  final WebSocketChannel channel;

  const CarRCScreen(this.channel, {super.key});

  @override
  Widget build(BuildContext context) {
    bool light = false;
    return Scaffold(
      body: StreamBuilder(
        stream: Dio().getUri(Uri.parse("http://192.168.0.2/stream")).asStream(),
        builder: (context, snap) {
          if (snap.connectionState == ConnectionState.done) {
            return Stack(
              alignment: Alignment.center,
              children: [
                const GradientBackGround(),
                Card(
                  color: Colors.white,
                  margin: const EdgeInsets.all(5),
                  shape: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(16),
                  ),
                  elevation: 5,
                  child: Padding(
                    padding: const EdgeInsets.all(32.0),
                    child: Column(
                      mainAxisSize: MainAxisSize.min,
                      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                      children: [
                        const Text(
                          "Error em conexão",
                          style: TextStyle(color: Colors.red),
                        ),
                        ElevatedButton(
                          child: const Text("Voltar"),
                          onPressed: () {
                            channel.sink.close();
                          },
                        ),
                      ],
                    ),
                  ),
                )
              ],
            );
          }
          if (snap.connectionState == ConnectionState.active) {
            return Stack(
              alignment: Alignment.center,
              children: [
                SizedBox(
                  height: MediaQuery.of(context).size.height,
                  width: MediaQuery.of(context).size.width,
                  child: Image.network(
                    snap.data!.data,
                    scale: 0.5,
                    gaplessPlayback: true,
                    fit: BoxFit.fill,
                  ),
                ),
                Positioned(
                  left: 70,
                  bottom: 50,
                  child: IconButton(
                    icon: const Icon(Icons.car_rental),
                    onPressed: () {
                      channel.sink.add(
                          '{"angle":90.0,"aceleration":100.0,"light":1,"gear":D}');
                    },
                  ),
                ),
                Positioned(
                  right: (MediaQuery.of(context).size.width - 30) / 2,
                  bottom: 70,
                  child: Card(
                    color: Colors.transparent,
                    shape: OutlineInputBorder(
                        borderRadius: BorderRadius.circular(50)),
                    elevation: 5,
                    child: IconButton(
                      icon: const Icon(Icons.home),
                      onPressed: () {
                        channel.sink.close();
                        Navigator.of(context).pop();
                      },
                    ),
                  ),
                ),
                Positioned(
                  right: 70,
                  bottom: 50,
                  child: SizedBox(
                    height: 100,
                    child: GestureDetector(
                      child: Card(
                          color: Colors.transparent,
                          shape: OutlineInputBorder(
                              borderRadius: BorderRadius.circular(50)),
                          elevation: 5,
                          child: LightChart(light)),
                      onTap: () {
                        channel.sink.add("A0,B0,R0,L${light ? 1 : 0}");
                      },
                    ),
                  ),
                ),
              ],
            );
          } else {
            return Stack(
              alignment: Alignment.center,
              children: [
                const GradientBackGround(),
                Card(
                  color: Colors.white,
                  margin: const EdgeInsets.all(5),
                  shape: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(16),
                  ),
                  elevation: 5,
                  child: Padding(
                    padding: const EdgeInsets.all(32.0),
                    child: Column(
                      mainAxisSize: MainAxisSize.min,
                      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                      children: const [
                        Text(
                          "Esperando",
                          style: TextStyle(color: Colors.red),
                        ),
                        SizedBox(
                          height: 25,
                          child: CircularProgressIndicator(
                            color: Colors.white,
                          ),
                        ),
                      ],
                    ),
                  ),
                )
              ],
            );
          }
        },
      ),
    );
  }
}
