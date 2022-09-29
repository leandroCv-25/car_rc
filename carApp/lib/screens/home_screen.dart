import 'package:car/screens/car_rc.dart';
import 'package:flutter/material.dart';
import 'package:web_socket_channel/io.dart';

class HomeScreen extends StatelessWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(
          "Carrinho",
          style: Theme.of(context).textTheme.headline5,
        ),
        centerTitle: true,
        elevation: 0,
      ),
      body: CarRCScreen(
          IOWebSocketChannel.connect(Uri.parse('ws://192.168.0.1/car'))),
    );
  }
}
