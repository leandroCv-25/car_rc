import 'package:flutter/material.dart';

class LoadingWidget extends StatelessWidget {
  const LoadingWidget({super.key});

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        mainAxisAlignment: MainAxisAlignment.spaceEvenly,
        crossAxisAlignment: CrossAxisAlignment.center,
        children: const [
          SizedBox(height: 16),
          CircleAvatar(
            radius: 80,
            backgroundImage: AssetImage("images/toy.png"),
            backgroundColor: Colors.transparent,
          ),
          SizedBox(height: 16),
          CircularProgressIndicator(
            valueColor: AlwaysStoppedAnimation(Colors.purple),
          ),
          SizedBox(height: 16),
        ],
      ),
    );
  }
}
